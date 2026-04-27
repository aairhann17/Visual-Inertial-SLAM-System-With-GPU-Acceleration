#pragma once

#include <vector>

#include "utils/types.hpp"

namespace vislam::backend {

class Optimizer {
	public:
		void OptimizeTrajectory(std::vector<utils::Pose>& trajectory) const;
};

}  // namespace vislam::backend
