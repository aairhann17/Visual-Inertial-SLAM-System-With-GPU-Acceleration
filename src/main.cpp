#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "backend/optimizer.hpp"
#include "frontend/feature_detector.hpp"
#include "frontend/imu_preintegration.hpp"
#include "mapping/map.hpp"
#include "mapping/triangulation.hpp"
#include "utils/dataset_loader.hpp"
#include "utils/logging.hpp"

namespace {

struct RunConfig {
	std::string dataset {"scaffold"};
	std::string path {};
	std::string output_path {"output/trajectory.txt"};
	int max_frames {300};
};

bool ParseInt(const std::string& value, int* out) {
	try {
		*out = std::stoi(value);
		return true;
	} catch (...) {
		return false;
	}
}

bool ParseArgs(const int argc, char** argv, RunConfig* config) {
	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];
		if (arg == "--dataset" && i + 1 < argc) {
			config->dataset = argv[++i];
		} else if (arg == "--path" && i + 1 < argc) {
			config->path = argv[++i];
		} else if (arg == "--output" && i + 1 < argc) {
			config->output_path = argv[++i];
		} else if (arg == "--max-frames" && i + 1 < argc) {
			if (!ParseInt(argv[++i], &config->max_frames)) {
				return false;
			}
		} else if (arg == "--help") {
			std::cout << "Usage: vislam_app [--dataset scaffold|euroc] [--path <dataset_dir>] "
						 "[--output <trajectory_path>] [--max-frames <N>]\n";
			return false;
		}
	}
	return true;
}

bool WriteTrajectory(
	const std::string& output_path,
	const std::vector<std::pair<double, vislam::utils::Pose>>& trajectory) {
	const std::filesystem::path out_path(output_path);
	if (out_path.has_parent_path()) {
		std::error_code ec;
		std::filesystem::create_directories(out_path.parent_path(), ec);
	}

	std::ofstream out(output_path);
	if (!out.is_open()) {
		return false;
	}

	for (const auto& row : trajectory) {
		const auto& pose = row.second;
		out << row.first << " "
			<< pose.position.x << " " << pose.position.y << " " << pose.position.z << " "
			<< pose.orientation.x << " " << pose.orientation.y << " " << pose.orientation.z << " "
			<< pose.orientation.w << "\n";
	}
	return true;
}

int RunScaffold() {
	using namespace vislam;

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
	return 0;
}

int RunEuroc(const RunConfig& config) {
	using namespace vislam;

	if (config.path.empty()) {
		utils::Log(utils::LogLevel::kError, "--path is required for --dataset euroc");
		return 1;
	}

	utils::DatasetLoader loader(config.path);
	const auto images = loader.LoadEurocCam0();
	const auto imus = loader.LoadEurocImu0();
	if (images.empty() || imus.empty()) {
		utils::Log(utils::LogLevel::kError, "Failed to load EuRoC camera or IMU data from provided path");
		return 1;
	}

	frontend::ImuPreintegration preint;
	std::size_t imu_idx = 0;
	std::vector<std::pair<double, utils::Pose>> trajectory;
	trajectory.reserve(static_cast<std::size_t>(std::min<int>(config.max_frames, static_cast<int>(images.size()))));

	const int frame_count = std::min<int>(config.max_frames, static_cast<int>(images.size()));
	for (int i = 0; i < frame_count; ++i) {
		const double frame_ts = images[static_cast<std::size_t>(i)].timestamp_s;
		while (imu_idx < imus.size() && imus[imu_idx].timestamp_s <= frame_ts) {
			preint.Integrate(imus[imu_idx]);
			++imu_idx;
		}

		trajectory.push_back({frame_ts, preint.DeltaPose()});
	}

	if (!WriteTrajectory(config.output_path, trajectory)) {
		utils::Log(utils::LogLevel::kError, "Failed to write trajectory to output file");
		return 1;
	}

	utils::Log(utils::LogLevel::kInfo, "EuRoC run complete");
	std::cout << "Loaded images: " << images.size() << "\n";
	std::cout << "Loaded imu: " << imus.size() << "\n";
	std::cout << "Written poses: " << trajectory.size() << "\n";
	std::cout << "Trajectory output: " << config.output_path << "\n";
	return 0;
}

}  // namespace

int main(int argc, char** argv) {
	using namespace vislam;

	utils::Log(utils::LogLevel::kInfo, "Starting Visual-Inertial SLAM app");
	RunConfig config;
	if (!ParseArgs(argc, argv, &config)) {
		return 1;
	}

	int code = 0;
	if (config.dataset == "euroc") {
		code = RunEuroc(config);
	} else {
		code = RunScaffold();
	}

	utils::Log(utils::LogLevel::kInfo, "Run complete");
	return code;
}
