# geoCFD

<img src="https://user-images.githubusercontent.com/72781910/192777906-37361ee5-a656-4a9a-8784-5c9302c6b2e9.PNG" width="310" height="200">  <img src="https://user-images.githubusercontent.com/72781910/192778310-c14abb50-e899-42e2-8acc-d7f76a0e31e2.PNG" width="310" height="200">

It's a `x64-windows10` version of [geocfd-Ubuntu](https://github.com/SEUZFY/geocfd-Ubuntu). Further development may be performed in this project since a discovery has been found 
that keeping using `WSL-Ubuntu` will occupy more and more space in C drive.

**Purpose**: Process geometry for cfd simulation.

`Now`:

- Read two adjacent buildings, process repeated vertices and build two `nef polyhedra`.

- Union two `nef polyhedra` into one `big nef polyhedron`.

- Export the `big nef polyhedron` as `.cityjson` file(with no repeated vertices) and visualise it in [ninja](https://ninja.cityjson.org/), observe its `exterior` and `interior`

  <img src="https://user-images.githubusercontent.com/72781910/192778523-577a7e85-21a1-4729-aa1f-a55e310e317f.PNG" width="310" height="200">  <img src="https://user-images.githubusercontent.com/72781910/192778715-af57768e-08d0-467c-8247-53708fa147b8.PNG" width="310" height="200">

- Get the `convex hull` of the `big nef polyhedron` and visualise it in [ninja](https://ninja.cityjson.org/), observe its `exterior` and `interior`

  <img src="https://user-images.githubusercontent.com/72781910/192779009-1fd55a91-ff85-4035-931b-347568eb1f3d.PNG" width="310" height="200">  <img src="https://user-images.githubusercontent.com/72781910/192779087-387b8762-cf13-4bed-a636-45b1e362d241.PNG" width="310" height="200">
  
- Perform [3D Minkowski Sum](https://doc.cgal.org/latest/Minkowski_sum_3/index.html#Chapter_3D_Minkowski_Sum_of_Polyhedra) and visualize the result in [ninja](https://ninja.cityjson.org/).

  <img src="https://user-images.githubusercontent.com/72781910/193134598-386e8a58-372b-4831-ae03-1005d882a514.PNG" width="310" height="230">  <img src="https://user-images.githubusercontent.com/72781910/193134833-f4fa5db0-2e30-4cea-83b1-255566c20399.PNG" width="310" height="230">

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

## attention

Using `CGAL::Polyhedron_3` can be tricky, since `Polyhedron_builder` doesn't like repeated vertices, and even if it works
with repeatness, the created `Polyhedron_3` is NOT closed(and thus can not be converted to `Nef_polyhedron`).
Thus extra care needs to be taken when creating `Polyhedron_3`.

## Other platforms

If you use other platforms (such as `Linux` or `MacOS`), you can refer to `CMakeLists.txt` file and use it to build a `CMake` project using `src`, `include` and `data` folder.
