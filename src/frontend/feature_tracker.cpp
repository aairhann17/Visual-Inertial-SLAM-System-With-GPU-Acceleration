#include "frontend/feature_tracker.hpp"

#include <cmath>
#include <limits>

namespace vislam::frontend {

namespace {

double SquaredDistance(const utils::Vec2& a, const utils::Vec2& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

}  // namespace

FeatureTracker::FeatureTracker(const double max_pixel_distance)
    : max_pixel_distance_(max_pixel_distance) {}

std::vector<utils::FeatureObservation> FeatureTracker::Track(
    const std::vector<utils::FeatureObservation>& previous,
    const std::vector<utils::Vec2>& current) const {
    std::vector<utils::FeatureObservation> tracked;
    const double max_sq = max_pixel_distance_ * max_pixel_distance_;

    for (const auto& prev : previous) {
        double best_distance = std::numeric_limits<double>::max();
        utils::Vec2 best_match {};
        bool found = false;

        for (const auto& cur : current) {
            const double sq = SquaredDistance(prev.pixel, cur);
            if (sq < best_distance) {
                best_distance = sq;
                best_match = cur;
                found = true;
            }
        }

        if (found && best_distance <= max_sq) {
            tracked.push_back({prev.track_id, best_match});
        }
    }

    return tracked;
}

}  // namespace vislam::frontend
