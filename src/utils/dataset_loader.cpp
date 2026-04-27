#include "utils/dataset_loader.hpp"

#include <fstream>
#include <sstream>

namespace vislam::utils {

DatasetLoader::DatasetLoader(std::string root_path) : root_path_(std::move(root_path)) {}

std::vector<std::pair<double, std::string>> DatasetLoader::LoadImageTimestamps(
    const std::string& relative_list_file) const {
    std::vector<std::pair<double, std::string>> rows;
    std::ifstream in(root_path_ + "/" + relative_list_file);
    if (!in.is_open()) {
        return rows;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        double ts = 0.0;
        std::string path;
        if (iss >> ts >> path) {
            rows.emplace_back(ts, path);
        }
    }

    return rows;
}

std::vector<ImuMeasurement> DatasetLoader::LoadImuMeasurements(const std::string& relative_csv_file) const {
    std::vector<ImuMeasurement> rows;
    std::ifstream in(root_path_ + "/" + relative_csv_file);
    if (!in.is_open()) {
        return rows;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        ImuMeasurement m;
        if (iss >> m.timestamp_s >> m.accel_mps2.x >> m.accel_mps2.y >> m.accel_mps2.z >> m.gyro_rps.x >> m.gyro_rps.y >> m.gyro_rps.z) {
            rows.push_back(m);
        }
    }

    return rows;
}

}  // namespace vislam::utils
