#include "frontend/imu_preintegration.hpp"

namespace vislam::frontend {

void ImuPreintegration::Reset() {
    delta_pose_ = {};
    velocity_ = {};
    last_timestamp_s_ = 0.0;
    initialized_ = false;
}

void ImuPreintegration::Integrate(const utils::ImuMeasurement& measurement) {
    if (!initialized_) {
        last_timestamp_s_ = measurement.timestamp_s;
        initialized_ = true;
        return;
    }

    const double dt = measurement.timestamp_s - last_timestamp_s_;
    if (dt <= 0.0) {
        return;
    }

    velocity_.x += measurement.accel_mps2.x * dt;
    velocity_.y += measurement.accel_mps2.y * dt;
    velocity_.z += measurement.accel_mps2.z * dt;

    delta_pose_.position.x += velocity_.x * dt;
    delta_pose_.position.y += velocity_.y * dt;
    delta_pose_.position.z += velocity_.z * dt;

    last_timestamp_s_ = measurement.timestamp_s;
}

const utils::Pose& ImuPreintegration::DeltaPose() const {
    return delta_pose_;
}

}  // namespace vislam::frontend
