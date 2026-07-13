#include "utils/dataset_loader.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> SplitCsv(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    return tokens;
}

}  // namespace

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

std::vector<TimestampedImage> DatasetLoader::LoadEurocCam0() const {
    std::vector<TimestampedImage> rows;
    std::ifstream in(root_path_ + "/mav0/cam0/data.csv");
    if (!in.is_open()) {
        return rows;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto fields = SplitCsv(line);
        if (fields.size() < 2) {
            continue;
        }

        try {
            const double timestamp_s = std::stod(fields[0]) * 1e-9;
            const std::string rel = std::string("mav0/cam0/data/") + fields[1];
            rows.push_back({timestamp_s, rel});
        } catch (...) {
            continue;
        }
    }

    std::sort(rows.begin(), rows.end(), [](const TimestampedImage& a, const TimestampedImage& b) {
        return a.timestamp_s < b.timestamp_s;
    });
    return rows;
}

std::vector<ImuMeasurement> DatasetLoader::LoadEurocImu0() const {
    std::vector<ImuMeasurement> rows;
    std::ifstream in(root_path_ + "/mav0/imu0/data.csv");
    if (!in.is_open()) {
        return rows;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto fields = SplitCsv(line);
        if (fields.size() < 7) {
            continue;
        }

        try {
            ImuMeasurement m;
            m.timestamp_s = std::stod(fields[0]) * 1e-9;
            m.gyro_rps.x = std::stod(fields[1]);
            m.gyro_rps.y = std::stod(fields[2]);
            m.gyro_rps.z = std::stod(fields[3]);
            m.accel_mps2.x = std::stod(fields[4]);
            m.accel_mps2.y = std::stod(fields[5]);
            m.accel_mps2.z = std::stod(fields[6]);
            rows.push_back(m);
        } catch (...) {
            continue;
        }
    }

    std::sort(rows.begin(), rows.end(), [](const ImuMeasurement& a, const ImuMeasurement& b) {
        return a.timestamp_s < b.timestamp_s;
    });
    return rows;
}

}  // namespace vislam::utils
