## Logic

### JsonHandler.hpp
Responsible for taking care of the input `.cityjson` file and store the necessary information(i.e., `Solid`, `Shell`, `Face`, `Vertices` of one building(part)).

### JsonWriter.hpp
Responsible for writing the result to a `.cityjson` file so that the result can be visualised in [ninja](https://ninja.cityjson.org/). It should be noted that users can choose to export the **exterior** or **interior** of the result buidling.

### Polyhedron.hpp
Responsible for

- **(a)** taking care of `repeatness` in the dataset - `Polyhedron builder` doesn't like repeated vertices.
    
- **(b)** passing the **correct** geometry information of a building(part) to `Polyhedron builder`.
    
- **(c)** building the `CGAL::Polyhedron_3` for each building(part).
    
- **(d)** converting the `CGAL::Polyhedron_3` to `CGAL::Nef_polyhedron_3` if `CGAL::Polyhedron_3` is `closed` otherwise use the `convex hull` instead.

- **(e)** unioning all the `CGAL::Nef_polyhedron_3` into one big `CGAL::Nef_polyhedron_3` via CSG operations.

  - **(e1)** applying the `minkowski sum` first on each `CGAL::Nef_polyhedron_3` and then performing the union operation(to eliminate the small gaps and then to eliminate the internal faces).
        
  - **(e2)** getting the `convex hull` of the big `CGAL::Nef_polyhedron_3` to eliminate the internal faces.
    
- **(f)** extracting the geometry information(i.e. `vertices`, `faces`) of the big `CGAL::Nef_polyhedron_3` via a visitor pattern.
    
- **(g)** processing the obtained geometry information -> to write the result to `json` file correctly, the obtained geometry information needs to be further processed.

### main.cpp

The entry point of the whole program.

## note:

the `(e1)` and `(e2)` are two different approaches, they can be switched on/off by macros in the `main.cpp` file like so:

```cpp
//#define _ENABLE_CONVEX_HULL_ // switch on/off convex hull method
#define _ENABLE_MINKOWSKI_SUM_ // switch on/off minkowski sum method -> active by default
```
