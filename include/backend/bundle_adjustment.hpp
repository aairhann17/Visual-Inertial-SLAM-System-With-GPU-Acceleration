#pragma once

#include <vector>

#include "utils/types.hpp"

namespace vislam::backend {

class BundleAdjustment {
	public:
		void Refine(
				std::vector<utils::Vec3>& map_points,
				std::vector<utils::Pose>& keyframe_poses) const;
};

}  // namespace vislam::backend
