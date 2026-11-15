#include "frontend/feature_detector.hpp"

int main() {
	vislam::frontend::FeatureDetector detector(200);

	const auto invalid = detector.Detect({}, 0, 480);
	if (!invalid.empty()) {
		return 1;
	}

	const auto features = detector.Detect({}, 640, 480);
	if (features.empty() || features.size() > 200) {
		return 1;
	}

	for (const auto& feature : features) {
		if (feature.x <= 0.0 || feature.x >= 640.0 || feature.y <= 0.0 || feature.y >= 480.0) {
			return 1;
		}
	}

	return 0;
}
