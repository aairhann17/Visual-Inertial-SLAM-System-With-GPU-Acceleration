#pragma once

#include "utils/types.hpp"

namespace vislam::frontend {

class ImuPreintegration {
	public:
		void Reset();
		void Integrate(const utils::ImuMeasurement& measurement);
		[[nodiscard]] const utils::Pose& DeltaPose() const;

	private:
		utils::Pose delta_pose_ {};
		utils::Vec3 velocity_ {};
		double last_timestamp_s_ {0.0};
		bool initialized_ {false};
};

}  // namespace vislam::frontend
