#pragma once

#include <vector>

#include "utils/types.hpp"

namespace vislam::backend {

struct PoseGraphEdge {
		std::size_t from {0};
		std::size_t to {0};
		utils::Pose relative_pose {};
};

class PoseGraph {
	public:
		void AddNode(const utils::Pose& pose);
		void AddEdge(const PoseGraphEdge& edge);
		void Optimize();

		[[nodiscard]] const std::vector<utils::Pose>& Nodes() const;

	private:
		std::vector<utils::Pose> nodes_;
		std::vector<PoseGraphEdge> edges_;
};

}  // namespace vislam::backend
