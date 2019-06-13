#include <algorithm>
#include <limits>
#include <sstream>
#include <vector>

struct svgplot {
	struct data {
		const double *x, *y;
		std::size_t n;
	};

	unsigned int width, height;
	struct {
		// min, max
		double x[2], y[2];
	} axes;

	std::vector<data> lines;

	svgplot() : width(800), height(600) {
		// limits start with "impossible" values that would
		// lose to real data in std::min() / std::max() comparison
		axes.x[0] = std::numeric_limits<double>::max();
		axes.x[1] = std::numeric_limits<double>::min();
		axes.y[0] = std::numeric_limits<double>::max();
		axes.y[1] = std::numeric_limits<double>::min();
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

		ss << "<?xml version=\"1.0\" standalone=\"no\"?>\n"
			<< "<svg"
			<< " width=\"" << width << "\" height=\"" << height << "\""
			// this makes it possible to draw in "plot" coordinates, except y axis is flipped
			<< " viewBox=\"0 0 1 1\" preserveAspectRatio=\"none\""
			<< " version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\""
			<< ">\n";

		// FIXME: physical units like em are unrealistic in (0, 0, 1, 1) viewport
		// so the whole purpose of having it like this is defeated
		// TODO: axes and stuff
		ss << "<rect"
			<< " width=\"100%\" height=\"100%\""
			<< " style=\"fill: none; stroke: black; stroke-width: .001em;\""
			<< "/>\n";

		for (size_t i = 0; i < lines.size(); i++) {
			ss << "<path style=\"fill: none; stroke: black; stroke-width: .001em;\" d=\"";
			for (size_t j = 0; j < lines[i].n; j++)
				ss << (j ? " L" : "M")
					<< (lines[i].x[j] - axes.x[0])/(axes.x[1] - axes.x[0])
					<< ','
					<< 1 - (lines[i].y[j] - axes.y[0])/(axes.y[1] - axes.y[0]); // y axis is flipped
			ss << "\"/>\n";
		}

		ss << "</svg>\n";

		return ss.str();
	}
};
