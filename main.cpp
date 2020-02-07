#include <iostream>
#include "svgplot.hpp"

int main() {
	double x[256] = {};
	double y1[sizeof(x)/sizeof(*x)];
	double y2[sizeof(x)/sizeof(*x)];

	for (size_t i = 0; i < sizeof(x)/sizeof(*x); ++i) {
		x[i] = (double)i / 32;
		y1[i] = sin(x[i]);
		y2[i] = 2 + sin(x[i]) + cos(3 * x[i]) + sin(5 * x[i]);
	}

	std::cout << svgplot()
		.add_line(x, y1, sizeof(x)/sizeof(*x))
		.add_line(x, y2, sizeof(x)/sizeof(*x))
		.draw();
}
