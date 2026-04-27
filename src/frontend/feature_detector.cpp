#include "frontend/feature_detector.hpp"

#include <algorithm>
#include <cmath>

namespace vislam::frontend {

FeatureDetector::FeatureDetector(const int max_features) : max_features_(std::max(1, max_features)) {}

std::vector<utils::Vec2> FeatureDetector::Detect(
    const std::vector<std::uint8_t>& /*grayscale*/,
    const int width,
    const int height) const {
    std::vector<utils::Vec2> features;
    if (width <= 0 || height <= 0) {
        return features;
    }

    const int grid = std::max(1, static_cast<int>(std::sqrt(static_cast<double>(max_features_))));
    const double step_x = static_cast<double>(width) / static_cast<double>(grid + 1);
    const double step_y = static_cast<double>(height) / static_cast<double>(grid + 1);

    for (int gy = 1; gy <= grid && static_cast<int>(features.size()) < max_features_; ++gy) {
        for (int gx = 1; gx <= grid && static_cast<int>(features.size()) < max_features_; ++gx) {
            features.push_back({step_x * gx, step_y * gy});
        }
    }

    return features;
}

}  // namespace vislam::frontend
