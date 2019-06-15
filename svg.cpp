#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <vector>

static double nicenum(double x) {
	// round down to power of 10
	double rnd = std::pow(10, std::floor(std::log(x)/std::log(10)));
	// how far is it from the original number?
	double diff = x - rnd, ret = rnd;
	// 1, 2, 5, 10 are "nice" multipliers, so try multiplying by them
	// in case it brings our estimate closer to original
	static const double mult[] = { 2., 5., 10. };
	for (unsigned i = 0; i < sizeof(mult)/sizeof(*mult); i++)
		if (std::abs(x - mult[i] * rnd) < diff) {
			diff = std::abs(x - mult[i] * rnd);
			ret = mult[i] * rnd;
		}
	return ret;
}

struct svgplot {
	struct data {
		const double *x, *y;
		std::size_t n;
	};

	unsigned int ticks; // how many ticks to place on the axes

	struct {
		double x[2], y[2]; // plot coordinates: {min, max}
	} range;

	enum mar { bottom, left, top, right, mar_max };
	double margins[mar_max]; // fraction

	std::vector<data> lines;

	svgplot() : ticks(4) {
		// limits start with "impossible" values that would
		// lose to real data in std::min() / std::max() comparison
		range.x[0] = std::numeric_limits<double>::max();
		range.x[1] = std::numeric_limits<double>::min();
		range.y[0] = std::numeric_limits<double>::max();
		range.y[1] = std::numeric_limits<double>::min();

		margins[bottom] = margins[left] = .10;
		margins[top] = margins[right] = .05;
	}

	svgplot & add_line(const double * x, const double * y, std::size_t n) {
		// update ranges
		range.x[0] = std::min(range.x[0], *std::min_element(x, x+n));
		range.y[0] = std::min(range.y[0], *std::min_element(y, y+n));
		range.x[1] = std::max(range.x[1], *std::max_element(x, x+n));
		range.y[1] = std::max(range.y[1], *std::max_element(y, y+n));
		// store the pointers
		data l = {x, y, n};
		lines.push_back(l);
		// allow chaining calls
		return *this;
	}

	std::string draw() {
		std::stringstream ss;
		ss.imbue(std::locale("C")); // prevent locale-related float formatting problems

		ss << "<svg"
			" version=\"1.2\" baseProfile=\"tiny\" xmlns=\"http://www.w3.org/2000/svg\""
			" viewBox=\"" << -margins[left] << ' ' << -margins[top] << ' '
			<< 1 + margins[right] << ' ' << 1 + margins[bottom] << "\""
			">\n"
			"<style>\n"
			"path { fill: none; stroke: black; stroke-width: 1px; }\n"
			"text { font-size: " << margins[bottom] / 2 << "px; }\n"
			"</style>\n"
		;

		// the plot area is just a series of lines, and there's a very
		// effective representation for that; who knew?
		ss << "<path vector-effect=\"non-scaling-stroke\" d=\"\n";

		// Tufte-style axes:
		// 1. Give the plot some space (start at outward "nice" tick position)
		double dx = nicenum((range.x[1]-range.x[0])/(ticks - 1)),
			dy = nicenum((range.y[1]-range.y[0])/(ticks - 1));
		struct { double x[2], y[2]; } axes = {
			{ dx * floor(range.x[0] / dx), dx * ceil(range.x[1] / dx) },
			{ dy * floor(range.y[0] / dy), dy * ceil(range.y[1] / dy) }
		};
		// 2. Make the axes cover the actual data range (not extend to [0,1])
		ss << "M" << (range.x[0] - axes.x[0])/(axes.x[1] - axes.x[0]) << ",1"
			"L" << (range.x[1] - axes.x[0])/(axes.x[1] - axes.x[0]) << ",1"
			"M0," << (axes.y[1] - range.y[0])/(axes.y[1] - axes.y[0])
			<< "L0," << (axes.y[1] - range.y[1])/(axes.y[1] - axes.y[0])
			<< "\n"
		;
		// 3. Make ticks at "nice" positions (multiples of dx, dy)
		for (double x = dx * ceil(range.x[0] / dx); x <= dx * floor(range.x[1] / dx); x += dx)
			ss << "M" << (x - axes.x[0])/(axes.x[1] - axes.x[0]) << ",1"
				"L" << (x - axes.x[0])/(axes.x[1] - axes.x[0]) << ",1.01";
		for (double y = dy * ceil(range.y[0] / dy); y <= dy * floor(range.y[1] / dy); y += dy)
			ss << "M0," << (axes.y[1] - y)/(axes.y[1] - axes.y[0])
				<< "L-.01," << (axes.y[1] - y)/(axes.y[1] - axes.y[0]);
		ss << "\n";

		// now draw the actual lines
		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 0; j < lines[i].n; j++)
				ss << (j ? 'L' : 'M')
					<< (lines[i].x[j] - axes.x[0])/(axes.x[1] - axes.x[0]) << ','
					<< (axes.y[1] - lines[i].y[j])/(axes.y[1] - axes.y[0]);
			ss << '\n';
		}
		ss << "\"/>\n";

		// oops, axis labels should be repeated here
		for (double x = dx * ceil(range.x[0] / dx); x <= dx * floor(range.x[1] / dx); x += dx)
			ss << "<text x=\"" << (x - axes.x[0])/(axes.x[1] - axes.x[0]) << "\""
				" y=\"" << 1 + margins[bottom] / 2 << "\">" << x << "</text>";
		ss << "\n";
		for (double y = dy * ceil(range.y[0] / dy); y <= dy * floor(range.y[1] / dy); y += dy)
			ss << "<text x=\"" << - margins[left] << "\""
				" y=\"" << (axes.y[1] - y)/(axes.y[1] - axes.y[0]) << "\">" << y << "</text>";
		ss << "\n";

		ss << "</svg>\n";

		return ss.str();
	}
};
