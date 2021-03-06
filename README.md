What is this?
=============

One day I needed to plot some data as SVG, with certain constraints:

* in C++, compatible with MSVC 10.0 **and** POSIX
* no screen or filesystem interaction

There aren't many C++ plotting libraries, and they all plot either to screen
or to files. [plplot](http://plplot.sourceforge.net/) came close because its
API can accept `FILE*` produced by standard C function `tmpfile()`, but this
function is broken in MSVCRT (tries to create files in the root directory of
the drive and fails with "permission denied"). [gnuplot](http://www.gnuplot.info/)
could be an option (it's possible to input commands via `stdin` and then read
the SVG from `stdout`), but cross-platform bi-directional piping of child
processes is a huge pain in the neck (and some deadlocks may be unavoidable,
depending on the program in question).

Since the plot I needed was *really* simple, this came to life. Features:

* Tufte-style line plots in black and white, multiple lines allowed, SVG1.2
  output
* Settings: margins, font size (in fractions of the plot dimensions),
  approximate number of "nice" tics on the axes, line stroke width (in CSS
  units)
* Subsampling: avoid drawing a line segment if it's shorter than given fraction
  of the plot dimensions

What does it look like?
=======================

![example plot](example.svg)

How do I use it?
================

```c++
#include <svgplot.hpp>

std::vector<double> x = ..., y = ...;

std::string svg = svgplot()
	.add_line(&x[0], &y[0], x.size())
	.set_ntics(7)
	.draw();
```
