#include "ros2/vislam_lifecycle_node.hpp"

#include <rclcpp/executors/single_threaded_executor.hpp>
#include <rclcpp/rclcpp.hpp>

#include "utils/logging.hpp"

namespace vislam::ros2 {

VislamLifecycleNode::VislamLifecycleNode(const rclcpp::NodeOptions& options)
	: rclcpp_lifecycle::LifecycleNode("vislam_lifecycle_node", options) {}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
VislamLifecycleNode::on_configure(const rclcpp_lifecycle::State& /*state*/) {
	configured_ = true;
	utils::Log(utils::LogLevel::kInfo, "ROS2 lifecycle: configured");
	return CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
VislamLifecycleNode::on_activate(const rclcpp_lifecycle::State& /*state*/) {
	if (!configured_) {
		utils::Log(utils::LogLevel::kError, "ROS2 lifecycle: activate requested before configure");
		return CallbackReturn::FAILURE;
	}

	active_ = true;
	utils::Log(utils::LogLevel::kInfo, "ROS2 lifecycle: active");
	return CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
VislamLifecycleNode::on_deactivate(const rclcpp_lifecycle::State& /*state*/) {
	active_ = false;
	utils::Log(utils::LogLevel::kInfo, "ROS2 lifecycle: inactive");
	return CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
VislamLifecycleNode::on_cleanup(const rclcpp_lifecycle::State& /*state*/) {
	configured_ = false;
	active_ = false;
	utils::Log(utils::LogLevel::kInfo, "ROS2 lifecycle: cleaned up");
	return CallbackReturn::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
VislamLifecycleNode::on_shutdown(const rclcpp_lifecycle::State& /*state*/) {
	active_ = false;
	utils::Log(utils::LogLevel::kInfo, "ROS2 lifecycle: shutdown");
	return CallbackReturn::SUCCESS;
}

}  // namespace vislam::ros2

int main(int argc, char** argv) {
	rclcpp::init(argc, argv);

	auto node = std::make_shared<vislam::ros2::VislamLifecycleNode>();
	rclcpp::executors::SingleThreadedExecutor executor;
	executor.add_node(node->get_node_base_interface());
	executor.spin();

	rclcpp::shutdown();
	return 0;
}

