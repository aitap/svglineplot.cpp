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

	double xscale(double x, const double range[2], const double margin[2]) {
		return margin[0] + (margin[1] - margin[0]) * (x - range[0])/(range[1] - range[0]);
	}

	double yscale(double y, const double range[2], const double margin[2]) {
		return margin[0] + (margin[1] - margin[0]) * (range[1] - y)/(range[1] - range[0]);
	}

	static std::string to_string(double x) {
		std::stringstream s;
		s.imbue(std::locale("C"));
		s << x;
		return s.str();
	}

	struct data {
		const double *x, *y;
		std::size_t n;
	};

	double width, height; // SVG user units
	unsigned int tics; // how many tics to place on the axes
	struct {
		double x[2], y[2]; // plot coordinates: {min, max}
	} range;
	double fontsize, strokewidth; // SVG user units
	double dmin; // Euclidean distance in ??? to subsample at

	std::vector<data> lines;

public:
	svgplot() {
		set_dimensions(800, 600);
		set_ntics(4);

		// limits start with "impossible" values that would
		// lose to real data in std::min() / std::max() comparison
		range.x[0] = std::numeric_limits<double>::max();
		range.x[1] = std::numeric_limits<double>::min();
		range.y[0] = std::numeric_limits<double>::max();
		range.y[1] = std::numeric_limits<double>::min();

		set_fontsize(20);
		set_strokewidth(1);
		set_subsample(0);
	}

	// plot a line defined by arrays x[n], y[n]
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

	// set dimensions of the plot in SVG user units
	svgplot & set_dimensions(double w, double h) {
		width = w;
		height = h;
		return *this;
	}

	// set the number of tics on the axes
	// (will be followed approximately, subject to "nice number" coordinates)
	svgplot & set_ntics(unsigned int tics_) {
		tics = tics_;
		return *this;
	}

	// font size, in SVG user units (NOTE: same as plot size)
	svgplot & set_fontsize(double fs) {
		fontsize = fs;
		return *this;
	}

	// line width for the plot and the axes, same units as plot size
	svgplot & set_strokewidth(double sw) {
		strokewidth = sw;
		return *this;
	}

	// Euclidean distance in SVG user units at which to perform subsampling
	svgplot & set_subsample(const double dmin_) {
		dmin = dmin_;
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
		std::vector<std::string> xlabs, ylabs;
		double xlablen = 0, ylablen = 0;
		for (double x = dx * ceil(range.x[0] / dx); x <= dx * floor(range.x[1] / dx); x += dx) {
			xtics.push_back(x);
			std::string lab = to_string(x);
			if (lab.length() > xlablen) xlablen = lab.length();
			xlabs.push_back(lab);
		}
		for (double y = dy * ceil(range.y[0] / dy); y <= dy * floor(range.y[1] / dy); y += dy) {
			ytics.push_back(y);
			std::string lab = to_string(y);
			if (lab.length() > ylablen) ylablen = lab.length();
			ylabs.push_back(lab);
		}
		// Sort-of calculate the margin SVG coordinates based on the length of axis labels (FIXME)
		struct { double x[2], y[2]; } margin = {
			{ xlablen * fontsize, width * .99  }, { height - fontsize, height * .01 }
		};

		std::stringstream ss;
		ss.imbue(std::locale("C")); // prevent locale-related float formatting problems

		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\""
			" viewBox=\"0 0 " << width << " " << height << "\""
			">\n"
			"<style>\n"
			"path { fill: none; stroke: black; stroke-width: " << strokewidth << "; }\n"
			"text { font-size: " << fontsize << "; }\n"
			"</style>\n"
		;

		// the plot area is just a series of lines, and there's a very
		// effective representation for that; who knew?
		ss << "<path d=\"\n";

		// Make the axes cover the actual data range (not extend to [0,1])
		ss << "M" << xscale(range.x[0], axes.x, margin.x) << "," << margin.y[0]
			<< "L" << xscale(range.x[1], axes.x, margin.x) << "," << margin.y[1]
			<< "M" << margin.x[0] << "," << yscale(range.y[0], axes.y, margin.y)
			<< "L" << margin.x[0] << "," << yscale(range.y[1], axes.y, margin.y)
		;
		// place tics on axes
		for (size_t i = 0; i < xtics.size(); ++i)
			ss << "M" << xscale(xtics[i], axes.x, margin.x) << "," << magrin.y[0]
				<< "L" << xscale(xtics[i], axes.x, margin.x) << "," << margin.y[0] * 1.01;
		for (size_t i = 0; i < ytics.size(); ++i)
			ss << "M0," << 1 - scale(ytics[i], axes.y)
				<< "L-.01," << 1 - scale(ytics[i], axes.y);
		ss << "\n";

		// now draw the actual lines
		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 0; j < lines[i].n; j++) {
				double lastdrawn[2],
					x = scale(lines[i].x[j], axes.x),
					y = 1 - scale(lines[i].y[j], axes.y);
				if (
					!j || // first point should always be drawn
					std::sqrt(
						std::pow(x - lastdrawn[0], 2) + std::pow(x - lastdrawn[1], 2)
					) > dmin // others are drawn if they are sufficiently far
				) {
					ss << (j ? 'L' : 'M') << x << ',' << y;
					lastdrawn[0] = x;
					lastdrawn[1] = y;
				}
			}
			ss << '\n';
		}
		ss << "\"/>\n";

		// path completed; add tic labels at remembered coordinates
		// NOTE: dx and dy are very approximate and might break down depending on the font
		for (size_t i = 0; i < xtics.size(); ++i)
			ss << "<text x=\"" << scale(xtics[i], axes.x) << "\" dx=\"-"
				<< xlabs[i].length() / 2. << "em\" y=\"1\" dy=\"1em\">"
				<< xlabs[i] << "</text>";
		ss << "\n";
		for (size_t i = 0; i < ytics.size(); ++i)
			ss << "<text dx=\"-" << ylabs[i].length() << "em\""
				" y=\"" << 1 - scale(ytics[i], axes.y) << "\" dy=\".5em\">"
				<< ylabs[i] << "</text>";
		ss << "\n";

		ss << "</svg>\n";

		return ss.str();
	}
};
