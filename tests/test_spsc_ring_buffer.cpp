#include "utils/spsc_ring_buffer.hpp"

int main() {
	vislam::utils::SpscRingBuffer<int, 4> buffer;

	if (!buffer.Empty() || !buffer.Push(1) || !buffer.Push(2) || !buffer.Push(3) || buffer.Push(4)) {
		return 1;
	}

	const auto first = buffer.Pop();
	if (!first.has_value() || *first != 1) {
		return 1;
	}

	const auto second = buffer.Pop();
	if (!second.has_value() || *second != 2) {
		return 1;
	}

	if (!buffer.Push(4)) {
		return 1;
	}
	const auto third = buffer.Pop();
	if (!third.has_value() || *third != 3) {
		return 1;
	}
	const auto fourth = buffer.Pop();
	if (!fourth.has_value() || *fourth != 4) {
		return 1;
	}

	if (!buffer.Empty()) {
		return 1;
	}
	return 0;
}
