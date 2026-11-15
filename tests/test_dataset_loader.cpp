#include "utils/dataset_loader.hpp"

int main() {
	const vislam::utils::DatasetLoader loader("path/that/does/not/exist");

	const auto images = loader.LoadEurocCam0();
	const auto imu = loader.LoadEurocImu0();

	if (!images.empty()) {
		return 1;
	}
	if (!imu.empty()) {
		return 1;
	}
	return 0;
}
