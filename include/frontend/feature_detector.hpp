#pragma once

#include <cstdint>
#include <vector>

#include "utils/types.hpp"

namespace vislam::frontend {

class FeatureDetector {
	public:
		explicit FeatureDetector(int max_features = 500);

		std::vector<utils::Vec2> Detect(const std::vector<std::uint8_t>& grayscale, int width, int height) const;

	private:
		int max_features_;
};

}  // namespace vislam::frontend
