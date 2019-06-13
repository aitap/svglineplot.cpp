#include <algorithm>
#include <limits>
#include <sstream>
#include <vector>

struct svgplot {
	struct data {
		const double *x, *y;
		std::size_t n;
	};

	unsigned int width, height; // em
	struct {
		// min, max
		double x[2], y[2]; // plot coordinates
	} axes;

	enum mar { bottom, left, top, right, mar_max };
	unsigned int margins[mar_max]; // em

	std::vector<data> lines;

	svgplot() : width(30), height(20) {
		// limits start with "impossible" values that would
		// lose to real data in std::min() / std::max() comparison
		axes.x[0] = std::numeric_limits<double>::max();
		axes.x[1] = std::numeric_limits<double>::min();
		axes.y[0] = std::numeric_limits<double>::max();
		axes.y[1] = std::numeric_limits<double>::min();

		margins[bottom] = margins[left] = 3;
		margins[top] = margins[right] = 1;
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
			" width=\"" << width << "em\" height=\"" << height << "em\""
			" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\""
			">\n"
			"<g style=\"fill: none; stroke: black; stroke-width: 1px;\">\n"
			"<svg" // plot area starts
			" x=\"" << margins[left] << "em\" y=\"" << margins[top] << "em\""
			" width=\"" << width - margins[right] - margins[left] << "em\""
			" height=\"" << height - margins[bottom] - margins[top] << "em\""
			">\n";

		// axes
		ss << "<line x1=\"0%\" y1=\"100%\" x2=\"0%\" y2=\"0%\"/>"
			"<line x1=\"0%\" y1=\"100%\" x2=\"100%\" y2=\"100%\"/>";

		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 1; j < lines[i].n; j++)
				// screw this, polylines and paths are unsuitable for relative or physical positions
				// let's do it the hard way, then
				ss << "<line"
					" x1=\"" << (lines[i].x[j-1] - axes.x[0])/(axes.x[1] - axes.x[0]) * 100 << "%\""
					" y1=\"" << (axes.y[1] - lines[i].y[j-1])/(axes.y[1] - axes.y[0]) * 100 << "%\""
					" x2=\"" << (lines[i].x[j] - axes.x[0])/(axes.x[1] - axes.x[0]) * 100 << "%\""
					" y2=\"" << (axes.y[1] - lines[i].y[j])/(axes.y[1] - axes.y[0]) * 100 << "%\""
					"/>";
			ss << "\n";
		}

		ss << "</svg>\n"; // plot area ends, can draw labels now

		ss << "</g></svg>\n";

		return ss.str();
	}
};
