#pragma once

#include <vector>

#include "utils/types.hpp"

namespace vislam::frontend {

class FeatureTracker {
	public:
		explicit FeatureTracker(double max_pixel_distance = 20.0);

		std::vector<utils::FeatureObservation> Track(
				const std::vector<utils::FeatureObservation>& previous,
				const std::vector<utils::Vec2>& current) const;

	private:
		double max_pixel_distance_;
};

}  // namespace vislam::frontend
