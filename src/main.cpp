#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

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

	const std::filesystem::path base_path(config.path);
	const std::filesystem::path cam_csv = base_path / "mav0" / "cam0" / "data.csv";
	const std::filesystem::path imu_csv = base_path / "mav0" / "imu0" / "data.csv";
	if (!std::filesystem::exists(cam_csv) || !std::filesystem::exists(imu_csv)) {
		if (!std::filesystem::exists(cam_csv)) {
			utils::Log(utils::LogLevel::kError, "Missing EuRoC file: " + cam_csv.string());
		}
		if (!std::filesystem::exists(imu_csv)) {
			utils::Log(utils::LogLevel::kError, "Missing EuRoC file: " + imu_csv.string());
		}
		utils::Log(utils::LogLevel::kError, "Expected dataset layout: <path>/mav0/cam0/data.csv and <path>/mav0/imu0/data.csv");
		return 1;
	}

	utils::DatasetLoader loader(config.path);
	const auto images = loader.LoadEurocCam0();
	const auto imus = loader.LoadEurocImu0();
	if (images.empty() || imus.empty()) {
		if (images.empty()) {
			utils::Log(utils::LogLevel::kError, "EuRoC camera index parsed as empty: " + cam_csv.string());
		}
		if (imus.empty()) {
			utils::Log(utils::LogLevel::kError, "EuRoC IMU index parsed as empty: " + imu_csv.string());
		}
		utils::Log(utils::LogLevel::kError, "Check CSV format and ensure files are not empty or header-only.");
		return 1;
	}

	std::vector<std::pair<double, utils::Pose>> trajectory;
	trajectory.reserve(static_cast<std::size_t>(std::min<int>(config.max_frames, static_cast<int>(images.size()))));

	cv::Mat previous_gray;
	std::vector<cv::Point2f> previous_points;
	utils::Pose current_pose {};
	bool has_previous_frame = false;
	const int frame_count = std::min<int>(config.max_frames, static_cast<int>(images.size()));

	for (int i = 0; i < frame_count; ++i) {
		const auto& image_info = images[static_cast<std::size_t>(i)];
		const std::filesystem::path image_path = base_path / image_info.relative_path;
		if (!std::filesystem::exists(image_path)) {
			utils::Log(utils::LogLevel::kError, "Missing image file: " + image_path.string());
			return 1;
		}

		cv::Mat gray = cv::imread(image_path.string(), cv::IMREAD_GRAYSCALE);
		if (gray.empty()) {
			utils::Log(utils::LogLevel::kError, "Failed to decode image: " + image_path.string());
			return 1;
		}

		if (!has_previous_frame) {
			cv::goodFeaturesToTrack(gray, previous_points, 200, 0.01, 10.0);
			previous_gray = gray.clone();
			has_previous_frame = true;
			trajectory.push_back({image_info.timestamp_s, current_pose});
			continue;
		}

		std::vector<cv::Point2f> current_points;
		std::vector<uchar> status;
		std::vector<float> err;
		cv::calcOpticalFlowPyrLK(previous_gray, gray, previous_points, current_points, status, err, cv::Size(21, 21), 3);

		std::vector<cv::Point2f> valid_prev;
		std::vector<cv::Point2f> valid_cur;
		valid_prev.reserve(current_points.size());
		valid_cur.reserve(current_points.size());
		for (std::size_t j = 0; j < current_points.size(); ++j) {
			if (status[j] && err[j] < 25.0f) {
				valid_prev.push_back(previous_points[j]);
				valid_cur.push_back(current_points[j]);
			}
		}

		if (!valid_cur.empty()) {
			cv::Point2f mean_delta(0.0f, 0.0f);
			for (std::size_t j = 0; j < valid_cur.size(); ++j) {
				mean_delta += valid_cur[j] - valid_prev[j];
			}
			mean_delta *= 1.0f / static_cast<float>(valid_cur.size());

			current_pose.position.x += mean_delta.x * 0.001;
			current_pose.position.y += mean_delta.y * 0.001;
			current_pose.position.z += 0.0;
		}

		previous_points = valid_cur;
		previous_gray = gray.clone();
		if (previous_points.empty()) {
			cv::goodFeaturesToTrack(gray, previous_points, 200, 0.01, 10.0);
		}

		trajectory.push_back({image_info.timestamp_s, current_pose});
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
