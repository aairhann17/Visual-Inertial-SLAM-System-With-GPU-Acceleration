#pragma once

#include <string>
#include <utility>
#include <vector>

#include "utils/types.hpp"

namespace vislam::utils {

class DatasetLoader {
	public:
		explicit DatasetLoader(std::string root_path);

		std::vector<std::pair<double, std::string>> LoadImageTimestamps(const std::string& relative_list_file) const;
		std::vector<ImuMeasurement> LoadImuMeasurements(const std::string& relative_csv_file) const;

	private:
		std::string root_path_;
};

}  // namespace vislam::utils
