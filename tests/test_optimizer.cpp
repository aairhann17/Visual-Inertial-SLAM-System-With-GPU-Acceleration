#include <cassert>
#include <vector>

#include "backend/optimizer.hpp"

int main() {
	std::vector<vislam::utils::Pose> trajectory(3);
	trajectory[0].position = {0.0, 0.0, 0.0};
	trajectory[1].position = {100.0, 0.0, 0.0};
	trajectory[2].position = {2.0, 0.0, 0.0};

	vislam::backend::Optimizer optimizer;
	optimizer.OptimizeTrajectory(trajectory);

	assert(trajectory[1].position.x < 100.0);
	return 0;
}
