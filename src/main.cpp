#include <iostream>

#include "backend/optimizer.hpp"
#include "frontend/feature_detector.hpp"
#include "frontend/imu_preintegration.hpp"
#include "mapping/map.hpp"
#include "mapping/triangulation.hpp"
#include "utils/logging.hpp"

int main() {
	using namespace vislam;

	utils::Log(utils::LogLevel::kInfo, "Starting Visual-Inertial SLAM scaffold app");

	frontend::FeatureDetector detector(200);
	auto features = detector.Detect({}, 640, 480);

	frontend::ImuPreintegration preint;
	preint.Integrate({0.0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}});
	preint.Integrate({0.01, {0.1, 0.0, 0.0}, {0.0, 0.0, 0.0}});

	mapping::Triangulation triangulation;
	const auto p3d = triangulation.Triangulate({320.0, 240.0}, {300.0, 240.0}, 0.1, 500.0);

	mapping::Map map;
	map.AddMapPoint(p3d);

	std::vector<utils::Pose> trajectory(3);
	trajectory[0].position = {0.0, 0.0, 0.0};
	trajectory[1].position = {1.0, 0.2, 0.1};
	trajectory[2].position = {2.0, 0.1, 0.0};

	backend::Optimizer optimizer;
	optimizer.OptimizeTrajectory(trajectory);

	std::cout << "Detected features: " << features.size() << "\n";
	std::cout << "Map points: " << map.MapPointCount() << "\n";
	std::cout << "Delta pose x: " << preint.DeltaPose().position.x << "\n";

	utils::Log(utils::LogLevel::kInfo, "Scaffold run complete");
	return 0;
}
