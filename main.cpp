#include <iostream>
#include "svg.cpp"

int main() {
	double x[] = {1, 2, 3, 4, 5, 6, 7, 8};
	double y[] = {.1, .2, .7, .4, .8, .6, 0, .2};

	double x1[] = {10, 11, 13, 14, 15, 16, 17, 18};
	double y1[] = {.7, .3, .1, .7, .9, .3, .9, .4, 2};

	std::cout << svgplot()
		.add_line(x, y, sizeof(x)/sizeof(*x))
		.add_line(x1, y1, sizeof(x1)/sizeof(*x1))
		.draw();
}
