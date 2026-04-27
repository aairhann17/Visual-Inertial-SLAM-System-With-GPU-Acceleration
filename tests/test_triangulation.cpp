#include <cassert>
#include <cmath>

#include "mapping/triangulation.hpp"

int main() {
	vislam::mapping::Triangulation triangulation;
	const auto p = triangulation.Triangulate({320.0, 240.0}, {310.0, 240.0}, 0.1, 500.0);

	const double expected_z = 500.0 * 0.1 / 10.0;
	assert(std::abs(p.z - expected_z) < 1e-9);
	return 0;
}
