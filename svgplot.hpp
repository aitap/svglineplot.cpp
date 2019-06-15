#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <vector>

class svgplot {
	static double nicenum(double x) {
		// Heckbert's algorithm
		// round down to power of 10
		double rnd = std::pow(10, std::floor(std::log(x)/std::log(10)));
		// how far is it from the original number?
		double diff = x - rnd, ret = rnd;
		// 1, 2, 5, 10 are "nice" multipliers, so try multiplying by them
		// in case it brings our estimate closer to original
		// (we include 10 to compensate for floor())
		static const double mult[] = { 2., 5., 10. };
		for (unsigned i = 0; i < sizeof(mult)/sizeof(*mult); i++)
			if (std::abs(x - mult[i] * rnd) < diff) {
				diff = std::abs(x - mult[i] * rnd);
				ret = mult[i] * rnd;
			}
		return ret;
	}

	static double scale(double x, const double range[2]) { // to [0,1]
		return (x - range[0])/(range[1] - range[0]);
	}

	static unsigned int length(double x) {
		// this is somewhat wasteful
		std::stringstream s;
		s.imbue(std::locale("C"));
		s << x;
		return s.str().length();
	}

	struct data {
		const double *x, *y;
		std::size_t n;
	};

	unsigned int tics; // how many tics to place on the axes
	struct {
		double x[2], y[2]; // plot coordinates: {min, max}
	} range;
	enum { bottom, left, top, right, n_margins_ };
	double margins[n_margins_]; // fraction of plot size
	double fontsize; // same
	std::string strokewidth; // CSS units

	std::vector<data> lines;

public:
	svgplot() {
		set_ntics(4);

		// limits start with "impossible" values that would
		// lose to real data in std::min() / std::max() comparison
		range.x[0] = std::numeric_limits<double>::max();
		range.x[1] = std::numeric_limits<double>::min();
		range.y[0] = std::numeric_limits<double>::max();
		range.y[1] = std::numeric_limits<double>::min();

		set_margins(.1, .15, .01, .01);
		set_fontsize(margins[bottom] / 2);
		set_strokewidth("1px");
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

	svgplot & set_margins(double bot_, double lef_, double top_, double rig_) {
		margins[bottom] = bot_;
		margins[left] = lef_;
		margins[top] = top_;
		margins[right] = rig_;
		return *this;
	}

	svgplot & set_ntics(unsigned int tics_) {
		tics = tics_;
		return *this;
	}

	svgplot & set_fontsize(double fs) {
		fontsize = fs;
		return *this;
	}

	svgplot & set_strokewidth(const std::string & sw) {
		strokewidth = sw;
		return *this;
	}

	std::string draw() {
		// Tufte's range-frame:
		// Give the plot some space (start at outward "nice" tic position)
		double dx = nicenum((range.x[1]-range.x[0])/(tics - 1)),
			dy = nicenum((range.y[1]-range.y[0])/(tics - 1));
		struct { double x[2], y[2]; } axes = {
			{ dx * floor(range.x[0] / dx), dx * ceil(range.x[1] / dx) },
			{ dy * floor(range.y[0] / dy), dy * ceil(range.y[1] / dy) }
		};
		// Tics at multiples of dx, dy
		std::vector<double> xtics, ytics;
		for (double x = dx * ceil(range.x[0] / dx); x <= dx * floor(range.x[1] / dx); x += dx)
			xtics.push_back(x);
		for (double y = dy * ceil(range.y[0] / dy); y <= dy * floor(range.y[1] / dy); y += dy)
			ytics.push_back(y);

		std::stringstream ss;
		ss.imbue(std::locale("C")); // prevent locale-related float formatting problems

		ss << "<svg"
			" version=\"1.2\" baseProfile=\"tiny\" xmlns=\"http://www.w3.org/2000/svg\""
			" viewBox=\"" << -margins[left] << ' ' << -margins[top]
			<< ' ' << 1 + margins[left] + margins[right]
			<< ' ' << 1 + margins[bottom] + margins[top] << "\""
			">\n"
			"<style>\n"
			"path { fill: none; stroke: black; stroke-width: " << strokewidth << "; }\n"
			"text { font-size: " << fontsize << "px; }\n"
			"</style>\n"
		;

		// the plot area is just a series of lines, and there's a very
		// effective representation for that; who knew?
		ss << "<path vector-effect=\"non-scaling-stroke\" d=\"\n";

		// Make the axes cover the actual data range (not extend to [0,1])
		ss << "M" << scale(range.x[0], axes.x) << ",1"
			"L" << scale(range.x[1], axes.x) << ",1"
			"M0," << 1 - scale(range.y[0], axes.y)
			<< "L0," << 1 - scale(range.y[1], axes.y)
		;
		// place tics on axes
		for (size_t i = 0; i < xtics.size(); ++i)
			ss << "M" << scale(xtics[i], axes.x) << ",1"
				"L" << scale(xtics[i], axes.x) << ",1.01";
		for (size_t i = 0; i < ytics.size(); ++i)
			ss << "M0," << 1 - scale(ytics[i], axes.y)
				<< "L-.01," << 1 - scale(ytics[i], axes.y);
		ss << "\n";

		// now draw the actual lines
		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 0; j < lines[i].n; j++)
				ss << (j ? 'L' : 'M')
					<< scale(lines[i].x[j], axes.x) << ','
					<< 1 - scale(lines[i].y[j], axes.y);
			ss << '\n';
		}
		ss << "\"/>\n";

		// path completed; add tic labels at remembered coordinates
		for (size_t i = 0; i < xtics.size(); ++i)
			ss << "<text x=\"" << scale(xtics[i], axes.x) << "\" dx=\"-"
				<< length(xtics[i]) / 2. << "em\" y=\"1\" dy=\"1em\">"
				<< xtics[i] << "</text>";
		ss << "\n";
		for (size_t i = 0; i < ytics.size(); ++i)
			ss << "<text x=\"-" << length(ytics[i]) << "em\""
				" y=\"" << 1 - scale(ytics[i], axes.y) << "\" dy=\".5em\">"
				<< ytics[i] << "</text>";
		ss << "\n";

		ss << "</svg>\n";

		return ss.str();
	}
};
