#include "backend/pose_graph.hpp"

namespace vislam::backend {

void PoseGraph::AddNode(const utils::Pose& pose) {
    nodes_.push_back(pose);
}

void PoseGraph::AddEdge(const PoseGraphEdge& edge) {
    edges_.push_back(edge);
}

void PoseGraph::Optimize() {
    for (const auto& edge : edges_) {
        if (edge.from >= nodes_.size() || edge.to >= nodes_.size()) {
            continue;
        }

        auto& to_pose = nodes_[edge.to];
        const auto& from_pose = nodes_[edge.from];

        to_pose.position.x = (to_pose.position.x + from_pose.position.x + edge.relative_pose.position.x) / 2.0;
        to_pose.position.y = (to_pose.position.y + from_pose.position.y + edge.relative_pose.position.y) / 2.0;
        to_pose.position.z = (to_pose.position.z + from_pose.position.z + edge.relative_pose.position.z) / 2.0;
    }
}

const std::vector<utils::Pose>& PoseGraph::Nodes() const {
    return nodes_;
}

}  // namespace vislam::backend
