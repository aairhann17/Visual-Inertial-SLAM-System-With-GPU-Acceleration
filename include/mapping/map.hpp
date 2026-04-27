#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "mapping/keyframe.hpp"
#include "utils/types.hpp"

namespace vislam::mapping {

class Map {
	public:
		void InsertKeyframe(const Keyframe& keyframe);
		void AddMapPoint(const utils::Vec3& map_point);

		[[nodiscard]] std::size_t KeyframeCount() const;
		[[nodiscard]] std::size_t MapPointCount() const;

	private:
		std::unordered_map<std::size_t, Keyframe> keyframes_;
		std::vector<utils::Vec3> map_points_;
};

}  // namespace vislam::mapping
