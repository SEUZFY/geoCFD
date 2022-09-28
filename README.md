# geoCFD

It's a x64-windows10 version of [geocfd-Ubuntu](https://github.com/SEUZFY/geocfd-Ubuntu). Further development may be performed in this project since a discovery has been found 
that keeping using `WSL-Ubuntu` will occupy more and more space in C drive.

**Purpose**: Process geometry for cfd simulation.

`Now`:

- Read two adjacent buildings, process repeated vertices and build two `nef polyhedra`.

- Union two `nef polyhedra` into one `big nef polyhedron`.

- Export the `big nef polyhedron` as `.cityjson` file(with no repeated vertices) and visualise it in [ninja](https://ninja.cityjson.org/), observe its `exterior` and `interior`

- Get the `convex hull` of the `big nef polyhedron` and visualise it in [ninja](https://ninja.cityjson.org/), observe its `exterior` and `interior`

`To do`:

- test more complicated buildings 

  - how much will the shape change?
  
  - [3D Minkowski Sum](https://doc.cgal.org/latest/Minkowski_sum_3/index.html#Chapter_3D_Minkowski_Sum_of_Polyhedra) - can we do sum in some specific direction?
 
 `long term`
 
  - `robust`
  
  - `val3dity`  - [validate](http://geovalidation.bk.tudelft.nl/val3dity/) the geometry
  
  - `validator` - [validate](https://validator.cityjson.org/) the `cityjson` file
  
  - `#include` - include multiple files, how to avoid possibly messy includings?

## Prerequisite

[CGAL](https://www.cgal.org/) - The version should be above `5.0` since we can use the `header-only`, which means we don't have to manually compile `CGAL`.

install `CGAL` via `vcpkg`:

check this -> [vcpkg Crash Course | C++ libraries simplified! A Visual Studio must-have!](https://www.youtube.com/watch?v=b7SdgK7Y510)

## How to use?

The files are built and executed on `x64-windows10` platform.

Clone this project and open it in `Visual Studio 2019` or newer version, you should be able to directly build and run.

## Compile info

Compiler: `MSVC`

Generator: `Ninja`

## Other platforms

If you use other platforms (such as `Linux` or `MacOS`), you can refer to `CMakeLists.txt` file and use it to build a `CMake` project using `src`, `include` and `data` folder.
