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

Since the plot I needed was *really* simple, this came to life.

What does it look like?
=======================

See the [example plot](example.svg). GitHub tries to "sanitize" it by removing
an essential attribute and breaking the plot in the process (I agree that's a
downside.)

The plots are "responsive", which means that they retain the stroke width
when scaled, but otherwise adapt to the viewport size, subject to their aspect
ratio.

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

TODO
====

- [ ] Aspect ratio as setting - as of now, it's 1:1 only
- [ ] Plot subsampling - it's too easy to produce a megabyte-sized SVG, most of which you wouldn't look at
