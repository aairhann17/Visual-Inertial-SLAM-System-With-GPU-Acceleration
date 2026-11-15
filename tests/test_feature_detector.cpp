#include <cassert>

#include "frontend/feature_detector.hpp"

int main() {
	vislam::frontend::FeatureDetector detector(200);

	const auto invalid = detector.Detect({}, 0, 480);
	assert(invalid.empty());

	const auto features = detector.Detect({}, 640, 480);
	assert(!features.empty());
	assert(features.size() <= 200);

	for (const auto& feature : features) {
		assert(feature.x > 0.0);
		assert(feature.x < 640.0);
		assert(feature.y > 0.0);
		assert(feature.y < 480.0);
	}

	return 0;
}

