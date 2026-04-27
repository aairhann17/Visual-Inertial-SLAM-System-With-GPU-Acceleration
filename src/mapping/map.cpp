#include "mapping/map.hpp"

namespace vislam::mapping {

void Map::InsertKeyframe(const Keyframe& keyframe) {
    keyframes_[keyframe.id] = keyframe;
}

void Map::AddMapPoint(const utils::Vec3& map_point) {
    map_points_.push_back(map_point);
}

std::size_t Map::KeyframeCount() const {
    return keyframes_.size();
}

std::size_t Map::MapPointCount() const {
    return map_points_.size();
}

}  // namespace vislam::mapping
