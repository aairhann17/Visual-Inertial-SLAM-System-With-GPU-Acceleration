#pragma once

#include <cstddef>

namespace vislam::frontend {

enum class TrackingHealthState {
	kInitializing,
	kTracking,
	kDegraded,
	kRecovering
};

struct TrackingHealthConfig {
	std::size_t min_visual_features {25};
	std::size_t max_consecutive_visual_drop {4};
	std::size_t max_consecutive_imu_drop {4};
	std::size_t recovery_stable_frames {3};
};

struct TrackingHealthDecision {
	TrackingHealthState state {TrackingHealthState::kInitializing};
	bool state_changed {false};
	bool should_reinitialize_features {false};
	const char* warning_code {nullptr};
};

class TrackingHealthStateMachine {
	public:
		explicit TrackingHealthStateMachine(TrackingHealthConfig config = {});

		TrackingHealthDecision Update(std::size_t visual_feature_count, std::size_t imu_samples_integrated);
		[[nodiscard]] TrackingHealthState State() const;

	private:
		TrackingHealthConfig config_;
		TrackingHealthState state_ {TrackingHealthState::kInitializing};
		std::size_t consecutive_visual_drop_ {0};
		std::size_t consecutive_imu_drop_ {0};
		std::size_t consecutive_recovery_good_ {0};
};

}  // namespace vislam::frontend

