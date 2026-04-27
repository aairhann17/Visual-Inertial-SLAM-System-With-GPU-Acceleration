#include <cuda_runtime.h>

__global__ void DummyFlowKernel(float* flow_u, float* flow_v, int count) {
	const int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < count) {
		flow_u[idx] = 0.0f;
		flow_v[idx] = 0.0f;
	}
}

extern "C" void RunCudaOpticalFlow(float* flow_u, float* flow_v, int count) {
	constexpr int kBlockSize = 256;
	const int grid = (count + kBlockSize - 1) / kBlockSize;
	DummyFlowKernel<<<grid, kBlockSize>>>(flow_u, flow_v, count);
}
