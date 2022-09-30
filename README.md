# geoCFD

<img src="https://user-images.githubusercontent.com/72781910/193364989-1d4eb5cb-a4a0-40b5-8af3-9b4405ca88f2.PNG" width=600>

Process geometry for cfd simulation.

It's a `x64-windows10` version of [geocfd-Ubuntu](https://github.com/SEUZFY/geocfd-Ubuntu). Further development may be performed in this project since a discovery has been found 
that keeping using `WSL-Ubuntu` will occupy more and more space in C drive.

`Now`:

- Read a building set(containing 23 buildings(buildingparts)), process repeated vertices and build `nef polyhedra`.

- Union `nef polyhedra` into one `big nef polyhedron`.

- Perform [3D Minkowski Sum](https://doc.cgal.org/latest/Minkowski_sum_3/index.html#Chapter_3D_Minkowski_Sum_of_Polyhedra).

- Get the [convex hull](https://en.wikipedia.org/wiki/Convex_hull) of the `big nef polyhedron` and visualise it in [ninja](https://ninja.cityjson.org/), observe its exterior and interior.

- Export the `big nef polyhedron` as `.cityjson` file(with no repeated vertices) and visualise it in [ninja](https://ninja.cityjson.org/), observe its `exterior` and `interior`.

  <img src="https://user-images.githubusercontent.com/72781910/193365248-fdf5899d-93ea-42e0-8d4e-84186ebc9ba2.PNG" width=600>
  
  <img src="https://user-images.githubusercontent.com/72781910/193365272-2d96000f-ca59-42a4-9801-111f28d7460b.PNG" width=600>

`To do`:

- test more complicated buildings 

  - how much will the shape change?
  
  - [3D Minkowski Sum](https://doc.cgal.org/latest/Minkowski_sum_3/index.html#Chapter_3D_Minkowski_Sum_of_Polyhedra) - can we do sum in some specific direction?

 `long term`
 
  - `robust`
  
  - `val3dity`  - [validate](http://geovalidation.bk.tudelft.nl/val3dity/) the geometry
  
  - `validator` - [validate](https://validator.cityjson.org/) the `cityjson` file
  
  - `#include` - include multiple files, how to avoid possibly messy includings?

  - `precompiled headers` - add `pch.h` file and use [cmake command for precompiled headers](https://cmake.org/cmake/help/latest/command/target_precompile_headers.html)
 
## Prerequisite

[CGAL](https://www.cgal.org/) - The version should be above `5.0` since we can use the `header-only`, which means we don't have to manually compile `CGAL`.

install `CGAL` via [vcpkg](https://vcpkg.io/en/index.html):

check this -> 

[Download CGAL for Windows](https://www.cgal.org/download/windows.html)

[vcpkg Crash Course | C++ libraries simplified! A Visual Studio must-have!](https://www.youtube.com/watch?v=b7SdgK7Y510)

## How to use?

This project is built and executed on a `windows10(64 bit)` platform.

Clone this project and open it in [Visual Studio](https://visualstudio.microsoft.com/) **2019** or newer version, you should be able to directly build and run.

## Compile info

C++ Standard: `C++ 11`

Compiler: `MSVC`

Generator: `Ninja`

## Attention

Using `CGAL::Polyhedron_3` can be tricky, since `Polyhedron_builder` doesn't like repeated vertices, and even if it works
with repeatness, the created `Polyhedron_3` is NOT closed(and thus can not be converted to `Nef_polyhedron`).
Thus extra care needs to be taken when creating `Polyhedron_3`.

## Benchmark

| building set| number of buildings | minkowski param | run time |
| ----------- | ------------------- | --------------- | -------- |
| 1           | 23                  | 1.0             | 29.7634s |

## Other platforms

If you use other platforms (such as `Linux` or `MacOS`), you can refer to `CMakeLists.txt` file and use it to build a `CMake` project using `src`, `include` and `data` folder.
