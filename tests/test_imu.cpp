#include <cassert>

#include "frontend/imu_preintegration.hpp"

int main() {
	vislam::frontend::ImuPreintegration preint;
	preint.Integrate({0.0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}});
	preint.Integrate({0.01, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}});

	const auto& dp = preint.DeltaPose();
	assert(dp.position.x > 0.0);
	return 0;
}
