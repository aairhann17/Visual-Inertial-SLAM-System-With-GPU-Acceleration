cmake_minimum_required(VERSION 3.18)

project(VisualInertialSLAM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VISLAM_BUILD_TESTS "Build unit-style test executables" ON)
option(VISLAM_ENABLE_CUDA "Build optional CUDA acceleration library" ON)

add_library(vislam_core STATIC
	src/frontend/feature_detector.cpp
	src/frontend/feature_tracker.cpp
	src/frontend/imu_preintegration.cpp
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

target_compile_features(vislam_core PUBLIC cxx_std_17)

add_executable(vislam_app src/main.cpp)
target_link_libraries(vislam_app PRIVATE vislam_core)

if(VISLAM_BUILD_TESTS)
	enable_testing()

	add_executable(test_imu tests/test_imu.cpp)
	target_link_libraries(test_imu PRIVATE vislam_core)
	add_test(NAME test_imu COMMAND test_imu)

	add_executable(test_triangulation tests/test_triangulation.cpp)
	target_link_libraries(test_triangulation PRIVATE vislam_core)
	add_test(NAME test_triangulation COMMAND test_triangulation)

	add_executable(test_optimizer tests/test_optimizer.cpp)
	target_link_libraries(test_optimizer PRIVATE vislam_core)
	add_test(NAME test_optimizer COMMAND test_optimizer)
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
