#include <cassert>

#include "frontend/tracking_health_state_machine.hpp"

int main() {
	vislam::frontend::TrackingHealthConfig config {};
	config.min_visual_features = 20;
	config.max_consecutive_visual_drop = 2;
	config.max_consecutive_imu_drop = 2;
	config.recovery_stable_frames = 2;
	vislam::frontend::TrackingHealthStateMachine machine(config);

	auto decision = machine.Update(40, 3);
	assert(decision.state == vislam::frontend::TrackingHealthState::kTracking);

	decision = machine.Update(5, 3);
	assert(decision.state == vislam::frontend::TrackingHealthState::kTracking);
	decision = machine.Update(5, 3);
	assert(decision.state == vislam::frontend::TrackingHealthState::kDegraded);
	assert(decision.should_reinitialize_features);

	decision = machine.Update(30, 4);
	assert(decision.state == vislam::frontend::TrackingHealthState::kRecovering);
	decision = machine.Update(30, 4);
	assert(decision.state == vislam::frontend::TrackingHealthState::kTracking);

	return 0;
}

