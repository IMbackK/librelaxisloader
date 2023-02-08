# librelaxisloader

librelaxisloader is a shared library that allows you to load data from RelaxIS3 files. This library supports only files with a DatabaseFormat of "1"

For questions or comments, please write to klemm@rhd-instruments.de

Full Online API documentation can be built with the "doc" target and is also avialable [here](http://uvos.xyz/kiss/librelaxisloaderdoc).
A PDF with API documentation can be found [here](http://uvos.xyz/kiss/librelaxisloader.pdf).

## Compile/Install

### Requirements

* git
* c99 capable compiler (GCC, CLANG)
* cmake 3.20 or later
* curl 7.64 or later
* (optinal) doxygen 1.8 or later to generate the documentation

### Procedure

In a console do:

* git clone https://git-ce.rwth-aachen.de/carl_philipp.klemm/librelaxisloader.git
* cd librelaxisloader
* mkdir build
* cd build
* cmake ..
* make
* sudo make install
