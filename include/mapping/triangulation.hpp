#pragma once

#include "utils/types.hpp"

namespace vislam::mapping {

class Triangulation {
	public:
		[[nodiscard]] utils::Vec3 Triangulate(
				const utils::Vec2& left_px,
				const utils::Vec2& right_px,
				double baseline_m,
				double focal_px) const;
};

}  // namespace vislam::mapping
