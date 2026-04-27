#include "mapping/triangulation.hpp"

#include <cmath>

namespace vislam::mapping {

utils::Vec3 Triangulation::Triangulate(
    const utils::Vec2& left_px,
    const utils::Vec2& right_px,
    const double baseline_m,
    const double focal_px) const {
    const double disparity = left_px.x - right_px.x;
    if (std::abs(disparity) < 1e-9 || baseline_m <= 0.0 || focal_px <= 0.0) {
        return {};
    }

    const double z = focal_px * baseline_m / disparity;
    const double x = left_px.x * z / focal_px;
    const double y = left_px.y * z / focal_px;
    return {x, y, z};
}

}  // namespace vislam::mapping
