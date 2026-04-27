#include "backend/optimizer.hpp"

namespace vislam::backend {

void Optimizer::OptimizeTrajectory(std::vector<utils::Pose>& trajectory) const {
    if (trajectory.size() < 3) {
        return;
    }

    for (std::size_t i = 1; i + 1 < trajectory.size(); ++i) {
        trajectory[i].position.x = (trajectory[i - 1].position.x + trajectory[i].position.x + trajectory[i + 1].position.x) / 3.0;
        trajectory[i].position.y = (trajectory[i - 1].position.y + trajectory[i].position.y + trajectory[i + 1].position.y) / 3.0;
        trajectory[i].position.z = (trajectory[i - 1].position.z + trajectory[i].position.z + trajectory[i + 1].position.z) / 3.0;
    }
}

}  // namespace vislam::backend
