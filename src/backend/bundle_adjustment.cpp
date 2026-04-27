#include "backend/bundle_adjustment.hpp"

namespace vislam::backend {

void BundleAdjustment::Refine(
    std::vector<utils::Vec3>& map_points,
    std::vector<utils::Pose>& keyframe_poses) const {
    for (auto& point : map_points) {
        point.x *= 0.999;
        point.y *= 0.999;
        point.z *= 0.999;
    }

    for (auto& pose : keyframe_poses) {
        pose.position.x *= 0.999;
        pose.position.y *= 0.999;
        pose.position.z *= 0.999;
    }
}

}  // namespace vislam::backend
