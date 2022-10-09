# geoCFD

<img width="558" alt="building_set_1_angle1" src="https://user-images.githubusercontent.com/72781910/194170661-8729cccf-e41a-4802-ab51-f71cba5e6d75.PNG">

Process geometry for cfd simulation.

It's a cross-platform project (currently tested on `x64-windows10` platform, see [geocfd-Ubuntu](https://github.com/SEUZFY/geocfd-Ubuntu) for the basic setting up on 
`wsl-ubuntu` platform). Further development are performed in this project since a discovery has been found that keeping using `WSL-Ubuntu` will occupy more and more space in C drive.

`Now`:

- support for all `LoD` levels (lod 1.2, lod 1.3, lod 2.2) in `cityjson` -> the code for `lod2.2` is currently available on [dev](https://github.com/SEUZFY/geoCFD/tree/dev) branch, not combined with lod1.2 and lod1.3 yet.

- Read a building set(containing 23 buildings(buildingparts)), process repeated vertices and build `nef polyhedra`.

- Union `nef polyhedra` into one `big nef polyhedron`, observe the original `exterior` and `internal faces`(highlighted in yellow).

  <img width="361" alt="set_1_exterior_m=0 1" src="https://user-images.githubusercontent.com/72781910/194174341-5d56621c-750a-4f32-b917-1840ff6e4313.png">   <img width="361" alt="set_1_interior_m=0 1" src="https://user-images.githubusercontent.com/72781910/194174083-0251a366-a61b-4357-a61d-042ea0110d1e.PNG">

	The interior of the adjacent buildings - `lod 2.2` (left) and `lod 1.3` (right)

- Perform [3D Minkowski Sum](https://doc.cgal.org/latest/Minkowski_sum_3/index.html#Chapter_3D_Minkowski_Sum_of_Polyhedra) with different parameters.

	**note**: this step works for `lod 1.2` and `lod 1.3`, but need to do some preprocessing for `lod 2.2`, see [robust](https://github.com/SEUZFY/geoCFD/tree/master#robust) section for details.

- Get the [convex hull](https://en.wikipedia.org/wiki/Convex_hull) of the `big nef polyhedron` and visualise it in [ninja](https://ninja.cityjson.org/), observe its exterior and interior (optional).

- Export the `big nef polyhedron` as `.cityjson` file(with no repeated vertices) and visualise it in [ninja](https://ninja.cityjson.org/), observe its `exterior` and `interior`.

  <img width="361" alt="set_1_exterior_m=0 1" src="https://user-images.githubusercontent.com/72781910/194171610-b7e8698d-98cb-47e1-a087-bae30da85817.PNG">   <img width="361" alt="set_1_interior_m=0 1" src="https://user-images.githubusercontent.com/72781910/194172354-8a20bc4b-f9f9-4116-9725-06738e7c0747.PNG">
  
	The interior of the result - `lod 2.2` (left) and `lod 1.3` (right)
	
- validate the result file via: 

	- `val3dity`  - [validate](http://geovalidation.bk.tudelft.nl/val3dity/) the geometry
  
  	- `validator` - [validate](https://validator.cityjson.org/) the `cityjson` file

`To do`:

  - how much will the shape change?
  
  - [3D Minkowski Sum](https://doc.cgal.org/latest/Minkowski_sum_3/index.html#Chapter_3D_Minkowski_Sum_of_Polyhedra) - can we do sum in some specific direction?

  - `robust` - see [robust](https://github.com/SEUZFY/geoCFD/tree/master#robust) section.

 `long term` 
  
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
1. **vertices repeatness**

	Using `CGAL::Polyhedron_3` can be tricky, since `Polyhedron_builder` doesn't like repeated vertices, and even if it workswith repeatness, the created `Polyhedron_3` is NOT closed(and thus can not be converted to `Nef_polyhedron`).Thus extra care needs to be taken when creating `Polyhedron_3`.

2. **geometry error**

	In `lod 2.2` geometries of buildings can get complicated, in our test there can be some geometry errors in the original dataset, for example:
	```console
	build nef for building: NL.IMBAG.Pand.0503100000018412-0

	CGAL::Polyhedron_incremental_builder_3<HDS>::
	lookup_halfedge(): input error: facet 29 has a self intersection at vertex 79.
	polyhedron closed? 0
	```
	A manual fix can be possible towards certain building set but not general, if we take the **universality** and **automation** into consideration, using `convex hull` to replace the corresponding building seems to be a good choice, yet this approach may lead to the compromisation of the original building shapes.
	
	There are also other issues in `lod 2.2`, see [robust](https://github.com/SEUZFY/geoCFD/tree/master#robust) section for details.

3. **DATA PATH**

	Currently the `absolute path` is used in this program, it should be noted that the path is kinda different on `windows` and `linux`
	system. This will be improved later.
	
## Robust

One important aspect is that `CGAL::Nef_polyhedron_3` will complain when the points of surfaces are not planar, which will lead to a `invalid` `Nef_polyhedron`.

A possible solution is demonstrated as below (it has been tested on building set 1):
```cpp
typedef CGAL::Simple_cartesian<float> Kernel;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron_3;

Polyhedron_3 polyhedron = load_my_polyhedron();
CGAL::Polygon_mesh_processing::triangulate_faces(polyhedron);
```
A triangulation process of the surfaces of a polyhedron can make points planar since there's always a plane that passes between any three points, no matter their locations but this is not guaranteed when having four or more points, which is pretty common in `lod 2.2`, among 23 `Nef polyhedra` from 23 buildings, 22 of them are
`invalid` and only 1 is `valid`.

It should however be noted that, triangulating process will modify the original geometry a bit (tiny change but for now not sure how to visualise / quantify the possible influence) and also have time cost.

Also we need to take the `geometry validity` of `CGAL::Polyhedron_3` into consideration. The `is_valid()` function only checks for `combinatorial validity` (for example in every half-edge should have an opposite oriented twin) but does not check the geometry correctness. In order to check it we can use `CGAL::Polygon_mesh_processing::do_intersect` to check for example if there are any intersections (need to verify and test).

Why do we need to check `geometry validity`? Because this could break the corresponding Nef polyhedron, for example, the corresponding Nef polyhedron is not valid.

possible solutions: allow users to switch on/off different robust check functions by defining different macros, for example:
```cpp
#define _POLYHEDRON_3_GEOMETRY_CHECK_
#define _POLYHEDRON_3_COMBINATORIAL_CHECK_
...
```
Also, since the triangulation process may have some cons, we can apply it on `lod 2.2` but not on `lod 1.2` and `lod 1.3`:
```cpp
if lod == 2.2
	triangulate the surfaces of polyhedron
	feed Nef with the triangulated polyhedron
...
```

## Benchmark

| building set| number of buildings |    lod level    | minkowski param | run time |
| :---------: | :-----------------: | :-------------: | :-------------: | :------: |
| 1           | 23                  | 1.3             | 1.0             | 29.7634s |
| 1           | 23                  | 1.3             | 0.5             | 29.9599s |
| 1           | 23                  | 1.3             | 0.1             | 33.8505s |
| 1           | 23                  | 1.2             | 0.1             | 29.8471s |
| 1           | 23                  | 2.2             | 0.1             | 256.724s |
| 1           | 23                  | 2.2             | not activated   | 18.3169s |

`with multi-threading`

| building set| number of buildings |    lod level    | minkowski param | run time |improved by|
| :---------: | :-----------------: | :-------------: | :-------------: | :------: |           |
| 1           | 23                  | 1.3             | 1.0             |          |           |
| 1           | 23                  | 1.3             | 0.5             |          |           |
| 1           | 23                  | 1.3             | 0.1             |          |           |
| 1           | 23                  | 1.2             | 0.1             |          |           |
| 1           | 23                  | 2.2             | 0.1             | 173.364s | 32.471%   |
| 1           | 23                  | 2.2             | not activated   | 13.0047s | 29.002%   |

## Other platforms

If you use other platforms (such as `Linux` or `MacOS`), you can refer to `CMakeLists.txt` file and use it to build a `CMake` project using `src`, `include` and `data` folder.
