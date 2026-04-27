#pragma once

#include <vector>

#include "utils/types.hpp"

namespace vislam::mapping {

struct Keyframe {
	std::size_t id {0};
	double timestamp_s {0.0};
	utils::Pose pose {};
	std::vector<utils::FeatureObservation> observations {};
};

}  // namespace vislam::mapping
