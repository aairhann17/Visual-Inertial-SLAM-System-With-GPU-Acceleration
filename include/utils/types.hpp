#pragma once

#include <cstdint>

namespace vislam::utils {

struct Vec2 {
	double x {0.0};
	double y {0.0};
};

struct Vec3 {
	double x {0.0};
	double y {0.0};
	double z {0.0};
};

struct Quaternion {
	double w {1.0};
	double x {0.0};
	double y {0.0};
	double z {0.0};
};

struct Pose {
	Vec3 position {};
	Quaternion orientation {};
};

struct ImuMeasurement {
	double timestamp_s {0.0};
	Vec3 accel_mps2 {};
	Vec3 gyro_rps {};
};

using TrackId = std::uint64_t;

struct FeatureObservation {
	TrackId track_id {0};
	Vec2 pixel {};
};

}  // namespace vislam::utils
