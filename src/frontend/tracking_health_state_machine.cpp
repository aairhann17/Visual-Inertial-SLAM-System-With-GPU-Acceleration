#include "frontend/tracking_health_state_machine.hpp"

#include <utility>

namespace vislam::frontend {

TrackingHealthStateMachine::TrackingHealthStateMachine(TrackingHealthConfig config)
	: config_(std::move(config)) {}

TrackingHealthDecision TrackingHealthStateMachine::Update(
	const std::size_t visual_feature_count,
	const std::size_t imu_samples_integrated) {
	const auto previous_state = state_;
	const bool visual_ok = visual_feature_count >= config_.min_visual_features;
	const bool imu_ok = imu_samples_integrated > 0;

	consecutive_visual_drop_ = visual_ok ? 0 : (consecutive_visual_drop_ + 1);
	consecutive_imu_drop_ = imu_ok ? 0 : (consecutive_imu_drop_ + 1);

	TrackingHealthDecision decision {};
	decision.state = state_;

	switch (state_) {
		case TrackingHealthState::kInitializing:
			if (visual_ok && imu_ok) {
				state_ = TrackingHealthState::kTracking;
			}
			break;
		case TrackingHealthState::kTracking:
			if (consecutive_visual_drop_ >= config_.max_consecutive_visual_drop) {
				state_ = TrackingHealthState::kDegraded;
				decision.should_reinitialize_features = true;
				decision.warning_code = "TRACKING_VISUAL_DROPOUT";
			} else if (consecutive_imu_drop_ >= config_.max_consecutive_imu_drop) {
				state_ = TrackingHealthState::kDegraded;
				decision.warning_code = "TRACKING_IMU_DROPOUT";
			}
			break;
		case TrackingHealthState::kDegraded:
			if (visual_ok && imu_ok) {
				state_ = TrackingHealthState::kRecovering;
				consecutive_recovery_good_ = 1;
			}
			break;
		case TrackingHealthState::kRecovering:
			if (visual_ok && imu_ok) {
				consecutive_recovery_good_ += 1;
				if (consecutive_recovery_good_ >= config_.recovery_stable_frames) {
					state_ = TrackingHealthState::kTracking;
					consecutive_recovery_good_ = 0;
				}
			} else {
				state_ = TrackingHealthState::kDegraded;
				consecutive_recovery_good_ = 0;
				decision.should_reinitialize_features = true;
				decision.warning_code = "TRACKING_RECOVERY_ABORTED";
			}
			break;
	}

	decision.state = state_;
	decision.state_changed = (state_ != previous_state);
	return decision;
}

TrackingHealthState TrackingHealthStateMachine::State() const {
	return state_;
}

}  // namespace vislam::frontend

