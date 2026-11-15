#pragma once

#include <memory>

#include <rclcpp_lifecycle/lifecycle_node.hpp>

namespace vislam::ros2 {

class VislamLifecycleNode : public rclcpp_lifecycle::LifecycleNode {
	public:
		explicit VislamLifecycleNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

	protected:
		CallbackReturn on_configure(const rclcpp_lifecycle::State& state) override;
		CallbackReturn on_activate(const rclcpp_lifecycle::State& state) override;
		CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state) override;
		CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state) override;
		CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state) override;

	private:
		bool configured_ {false};
		bool active_ {false};
};

}  // namespace vislam::ros2

