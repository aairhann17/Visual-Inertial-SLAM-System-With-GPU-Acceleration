add_library(vislam_gpu STATIC
	cuda_feature_extract.cu
	cuda_optical_flow.cu
)

target_include_directories(vislam_gpu
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)

set_target_properties(vislam_gpu PROPERTIES
	CUDA_STANDARD 17
	CUDA_STANDARD_REQUIRED ON
)
