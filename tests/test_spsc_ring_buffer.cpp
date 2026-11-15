#include <cassert>

#include "utils/spsc_ring_buffer.hpp"

int main() {
	vislam::utils::SpscRingBuffer<int, 4> buffer;

	assert(buffer.Empty());
	assert(buffer.Push(1));
	assert(buffer.Push(2));
	assert(buffer.Push(3));
	assert(!buffer.Push(4));

	const auto first = buffer.Pop();
	assert(first.has_value());
	assert(*first == 1);

	const auto second = buffer.Pop();
	assert(second.has_value());
	assert(*second == 2);

	assert(buffer.Push(4));
	const auto third = buffer.Pop();
	assert(third.has_value());
	assert(*third == 3);
	const auto fourth = buffer.Pop();
	assert(fourth.has_value());
	assert(*fourth == 4);

	assert(buffer.Empty());
	return 0;
}

