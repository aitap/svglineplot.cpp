#include <algorithm>
#include <limits>
#include <sstream>
#include <vector>

struct svgplot {
	struct data {
		const double *x, *y;
		std::size_t n;
	};

	unsigned int ticks; // how many ticks to place on the axes, TODO

	struct {
		double x[2], y[2]; // plot coordinates: {min, max}
	} axes;

	enum mar { bottom, left, top, right, mar_max };
	unsigned int margins[mar_max]; // %

	std::vector<data> lines;

	svgplot() : ticks(4) {
		// limits start with "impossible" values that would
		// lose to real data in std::min() / std::max() comparison
		axes.x[0] = std::numeric_limits<double>::max();
		axes.x[1] = std::numeric_limits<double>::min();
		axes.y[0] = std::numeric_limits<double>::max();
		axes.y[1] = std::numeric_limits<double>::min();

		margins[bottom] = margins[left] = 10;
		margins[top] = margins[right] = 5;
	}

	svgplot & add_line(const double * x, const double * y, std::size_t n) {
		// update ranges
		axes.x[0] = std::min(axes.x[0], *std::min_element(x, x+n));
		axes.y[0] = std::min(axes.y[0], *std::min_element(y, y+n));
		axes.x[1] = std::max(axes.x[1], *std::max_element(x, x+n));
		axes.y[1] = std::max(axes.y[1], *std::max_element(y, y+n));
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
			">\n"
			"<g style=\"fill: none; stroke: black; stroke-width: 1px;\">\n"
			// NOTE: plot area starts
			"<svg"
			" x=\"" << margins[left] << "%\" y=\"" << margins[top] << "%\""
			" width=\"" << 100 - margins[right] - margins[left] << "%\""
			" height=\"" << 100 - margins[bottom] - margins[top] << "%\""
			// plot coordinates: [0; 1] + a little extra to prevent antialiased lines from being cropped
			" viewBox=\"-.01 -.01 1.02 1.02\""
			">"
		;

		// the plot area is just a series of lines, and there's a very
		// effective representation for that; who knew?
		ss << "<path vector-effect=\"non-scaling-stroke\" d=\"\n";

		// TODO: Tufte-style axes (start and end on a "round" value)
		ss << "M0,1 L0,0 M0,1 L1,1\n";

		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 0; j < lines[i].n; j++)
				ss << (j ? 'L' : 'M')
					<< (lines[i].x[j] - axes.x[0])/(axes.x[1] - axes.x[0]) << ','
					<< (axes.y[1] - lines[i].y[j])/(axes.y[1] - axes.y[0]) << ' ';
			ss << '\n';
		}
		ss << "\"/></svg>\n"; // NOTE: plot area ends

		ss << "</g></svg>\n";

		return ss.str();
	}
};
