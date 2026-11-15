cmake_minimum_required(VERSION 3.18)

project(VisualInertialSLAM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VISLAM_ENABLE_STRICT_WARNINGS "Enable strict compiler warnings as errors" ON)
option(VISLAM_ENABLE_ROS2 "Build ROS2 lifecycle managed node target" OFF)
option(VISLAM_BUILD_TESTS "Build unit-style test executables" ON)
option(VISLAM_ENABLE_CUDA "Build optional CUDA acceleration library" ON)
option(VISLAM_USE_SPDLOG "Use spdlog-backed structured logging" ON)

if(NOT DEFINED OpenCV_DIR)
    if(DEFINED ENV{VCPKG_INSTALLED_DIR})
        set(OpenCV_DIR "$ENV{VCPKG_INSTALLED_DIR}/x64-windows/share/opencv4")
    elseif(EXISTS "C:/vcpkg/installed/x64-windows/share/opencv4/OpenCVConfig.cmake")
        set(OpenCV_DIR "C:/vcpkg/installed/x64-windows/share/opencv4")
    endif()
endif()

find_package(OpenCV REQUIRED CONFIG COMPONENTS core imgproc imgcodecs video videoio highgui)
if(VISLAM_USE_SPDLOG)
	find_package(spdlog QUIET)
endif()

add_library(vislam_core STATIC
	src/frontend/feature_detector.cpp
	src/frontend/feature_tracker.cpp
	src/frontend/imu_preintegration.cpp
	src/frontend/tracking_health_state_machine.cpp
	src/backend/optimizer.cpp
	src/backend/bundle_adjustment.cpp
	src/backend/pose_graph.cpp
	src/mapping/triangulation.cpp
	src/mapping/keyframe.cpp
	src/mapping/map.cpp
	src/utils/dataset_loader.cpp
)

target_include_directories(vislam_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vislam_core PUBLIC ${OpenCV_LIBS})
if(VISLAM_USE_SPDLOG AND spdlog_FOUND)
	if(TARGET spdlog::spdlog_header_only)
		target_link_libraries(vislam_core PUBLIC spdlog::spdlog_header_only)
	else()
		target_link_libraries(vislam_core PUBLIC spdlog::spdlog)
	endif()
	target_compile_definitions(vislam_core PUBLIC VISLAM_HAS_SPDLOG=1)
else()
	target_compile_definitions(vislam_core PUBLIC VISLAM_HAS_SPDLOG=0)
	if(VISLAM_USE_SPDLOG)
		message(WARNING "VISLAM_USE_SPDLOG=ON but spdlog was not found; falling back to std::cout logging.")
	endif()
endif()

function(vislam_enable_warnings target_name)
	if(NOT VISLAM_ENABLE_STRICT_WARNINGS)
		return()
	endif()

	if(MSVC)
		target_compile_options(${target_name} PRIVATE /W4 /WX /permissive-)
	else()
		target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Werror)
	endif()
endfunction()

add_executable(vislam_app src/main.cpp)
target_link_libraries(vislam_app PRIVATE vislam_core)
vislam_enable_warnings(vislam_core)
vislam_enable_warnings(vislam_app)

if(VISLAM_BUILD_TESTS)
	enable_testing()

	add_executable(test_imu tests/test_imu.cpp)
	target_link_libraries(test_imu PRIVATE vislam_core)
	vislam_enable_warnings(test_imu)
	add_test(NAME test_imu COMMAND test_imu)

	add_executable(test_triangulation tests/test_triangulation.cpp)
	target_link_libraries(test_triangulation PRIVATE vislam_core)
	vislam_enable_warnings(test_triangulation)
	add_test(NAME test_triangulation COMMAND test_triangulation)

	add_executable(test_optimizer tests/test_optimizer.cpp)
	target_link_libraries(test_optimizer PRIVATE vislam_core)
	vislam_enable_warnings(test_optimizer)
	add_test(NAME test_optimizer COMMAND test_optimizer)

	add_executable(test_feature_detector tests/test_feature_detector.cpp)
	target_link_libraries(test_feature_detector PRIVATE vislam_core)
	vislam_enable_warnings(test_feature_detector)
	add_test(NAME test_feature_detector COMMAND test_feature_detector)

	add_executable(test_pose_invariants tests/test_pose_invariants.cpp)
	target_link_libraries(test_pose_invariants PRIVATE vislam_core)
	vislam_enable_warnings(test_pose_invariants)
	add_test(NAME test_pose_invariants COMMAND test_pose_invariants)

	add_executable(test_dataset_loader tests/test_dataset_loader.cpp)
	target_link_libraries(test_dataset_loader PRIVATE vislam_core)
	vislam_enable_warnings(test_dataset_loader)
	add_test(NAME test_dataset_loader COMMAND test_dataset_loader)

	add_executable(test_spsc_ring_buffer tests/test_spsc_ring_buffer.cpp)
	target_link_libraries(test_spsc_ring_buffer PRIVATE vislam_core)
	vislam_enable_warnings(test_spsc_ring_buffer)
	add_test(NAME test_spsc_ring_buffer COMMAND test_spsc_ring_buffer)

	add_executable(test_tracking_health_state_machine tests/test_tracking_health_state_machine.cpp)
	target_link_libraries(test_tracking_health_state_machine PRIVATE vislam_core)
	vislam_enable_warnings(test_tracking_health_state_machine)
	add_test(NAME test_tracking_health_state_machine COMMAND test_tracking_health_state_machine)
endif()

if(VISLAM_ENABLE_CUDA)
	include(CheckLanguage)
	check_language(CUDA)

	if(CMAKE_CUDA_COMPILER)
		enable_language(CUDA)
		add_subdirectory(gpu)
		target_link_libraries(vislam_app PRIVATE vislam_gpu)
	else()
		message(WARNING "VISLAM_ENABLE_CUDA=ON but no CUDA compiler found; skipping gpu/ targets.")
	endif()
endif()

if(VISLAM_ENABLE_ROS2)
	find_package(rclcpp QUIET)
	find_package(rclcpp_lifecycle QUIET)

	if(rclcpp_FOUND AND rclcpp_lifecycle_FOUND)
		add_executable(vislam_ros2_lifecycle_node src/ros2/vislam_lifecycle_node.cpp)
		target_include_directories(vislam_ros2_lifecycle_node PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
		target_link_libraries(vislam_ros2_lifecycle_node PRIVATE vislam_core rclcpp::rclcpp rclcpp_lifecycle::rclcpp_lifecycle)
		vislam_enable_warnings(vislam_ros2_lifecycle_node)
	else()
		message(WARNING "VISLAM_ENABLE_ROS2=ON but ROS2 packages were not found; skipping vislam_ros2_lifecycle_node.")
	endif()
endif()
