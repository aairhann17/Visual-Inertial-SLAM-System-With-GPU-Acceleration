#include <cuda_runtime.h>

__global__ void DummyFeatureKernel(float* out_scores, int count) {
	const int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < count) {
		out_scores[idx] = 1.0f;
	}
}

extern "C" void RunCudaFeatureExtract(float* out_scores, int count) {
	constexpr int kBlockSize = 256;
	const int grid = (count + kBlockSize - 1) / kBlockSize;
	DummyFeatureKernel<<<grid, kBlockSize>>>(out_scores, count);
}
