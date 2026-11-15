#include <cassert>
#include <cmath>
#include <vector>

#include "backend/optimizer.hpp"

namespace {

double QuaternionNorm(const vislam::utils::Quaternion& q) {
	return std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

}  // namespace

int main() {
	std::vector<vislam::utils::Pose> trajectory(3);
	trajectory[0].position = {0.0, 0.0, 0.0};
	trajectory[1].position = {5.0, -1.0, 0.5};
	trajectory[2].position = {10.0, -2.0, 1.0};

	for (const auto& pose : trajectory) {
		assert(std::abs(QuaternionNorm(pose.orientation) - 1.0) < 1e-12);
	}

	vislam::backend::Optimizer optimizer;
	optimizer.OptimizeTrajectory(trajectory);

	for (const auto& pose : trajectory) {
		assert(std::abs(QuaternionNorm(pose.orientation) - 1.0) < 1e-12);
	}

	return 0;
}

