/*
* process geometry for cfd simulation
* main.cpp
*/


#include <CGAL/Polyhedron_3.h>
#include "JsonWriter.hpp"

#include<chrono> //Timer
#include<future> //async

#include "cmdline.h" // for cmd line parser
#include "MultiThread.hpp"



//#define DATA_PATH "D:\\SP\\geoCFD\\data" // specify the data path
//#define _ENABLE_CONVEX_HULL_ // switch on/off convex hull method
#define _ENABLE_MINKOWSKI_SUM_ // switch on/off minkowski sum method -> activated by default
#define _ENABLE_MULTI_THREADING_ // switch on/off multi-threading -> activated by default



/* user defined parameters --------------------------------------------------------------------------------------------------*/

/* triangulation */
bool _ENABLE_TRIANGULATION_ = false; // true - activate the triangulation process, false otherwise

/* lod level */
double lod = 1.3;

/* minkowski parameter */
double minkowski_param = 0.1;

/* user defined parameters --------------------------------------------------------------------------------------------------*/



/* optional parameters ------------------------------------------------------------------------------------------------------*/

/* number of adjacent buildings in one block */
unsigned int adjacency_size = 50;

/* optional parameters ------------------------------------------------------------------------------------------------------*/



// Timer class -> used for tracking the run time
struct Timer //for counting the time
{
	std::chrono::time_point<std::chrono::steady_clock>start, end;
	std::chrono::duration<float>duration;

	Timer() //set default value
	{
		start = end = std::chrono::high_resolution_clock::now();
		duration = end - start;
	}

	~Timer() // get the end value and print the duration time
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		std::cout << "Time: " << duration.count() << "s\n";
	}
};



// entry point
int main(int argc, char* argv[])
{
	std::cout << "test multi threading" << std::endl;

	std::string srcFile = "D:\\SP\\geoCFD\\data\\3dbag_v210908_fd2cee53_5907.json";
	std::ifstream input(srcFile);
	json j;
	input >> j;
	input.close();

	// get ids of adjacent buildings
	std::string adjacencyFile = "D:\\SP\\geoCFD\\data\\adjacency.txt";
	std::vector<std::string> adjacency;
	adjacency.reserve(adjacency_size);
	FileIO::read_adjacency_from_txt(adjacencyFile, adjacency);

	// read buildings
	std::vector<JsonHandler> jhandles;
	jhandles.reserve(adjacency_size); // use reserve() to avoid extra copies
	std::cout << "------------------------ building(part) info ------------------------\n";

	for (auto const& building_name : adjacency) // get each building
	{
		JsonHandler jhandle;
		jhandle.read_certain_building(j, building_name, lod); // read in the building
		jhandle.message();
		jhandles.emplace_back(jhandle); // add to the jhandlers vector
	}

	std::cout << "---------------------------------------------------------------------\n";

	/* begin counting */
	Timer timer; // count the run time
	
	MT::get_nefs_async(jhandles);
	std::cout << "nefs_ptr size: " << MT::m_nef_ptrs.size() << std::endl;

	// merging nefs into one big nef
	std::cout << "building big nef ..." << '\n';
	Nef_polyhedron big_nef;
	for (const auto& nef_ptr : MT::m_nef_ptrs) {
		big_nef += (*nef_ptr);
	}
	std::cout << "done" << '\n';

	MT::clean();
	
	
#ifdef _ENABLE_CONVEX_HULL_
	/* get the convex hull of the big_nef, use all cleaned vertices of all shells */
	// get cleaned vertices of shell_explorers[0] - the shell indicating the exterior of the big nef
	std::vector<Point_3>& convex_vertices = shell_explorers[0].cleaned_vertices;

	// build convex hull of the big nef
	Polyhedron convex_polyhedron; // define polyhedron to hold convex hull
	Nef_polyhedron convex_big_nef;
	CGAL::convex_hull_3(convex_vertices.begin(), convex_vertices.end(), convex_polyhedron);
	std::cout << "is convex closed? " << convex_polyhedron.is_closed() << '\n';
	if (convex_polyhedron.is_closed()) {
		std::cout << "build convex hull for the big nef...\n";
		Nef_polyhedron convex_nef(convex_polyhedron);
		convex_big_nef = convex_nef;
		std::cout << "build convex hull for the big nef done\n";
	}

	// process the convex big nef to make it available for output
	std::vector<Shell_explorer> convex_shell_explorers;
	NefProcessing::extract_nef_geometries(convex_big_nef, convex_shell_explorers);
	NefProcessing::process_shells_for_cityjson(convex_shell_explorers);
#endif


	// extracting geometries
	std::vector<Shell_explorer> shell_explorers; // store the extracted geometries
	NefProcessing::extract_nef_geometries(big_nef, shell_explorers); // extract geometries of the bignef
	NefProcessing::process_shells_for_cityjson(shell_explorers); // process shells for writing to cityjson


    // write file
	std::string path = "D:\\SP\\geoCFD\\data";
	std::string delimiter = "\\";
	JsonWriter jwrite;
	std::string writeFilename = "buildingset_1_multi_m=0.1.json";
	const Shell_explorer& shell = shell_explorers[0]; // which shell is going to be written to the file, 0 - exterior, 1 - interior
	std::cout << "writing the result to cityjson file...\n";
	jwrite.write_json_file(path + delimiter + writeFilename, shell, lod);

	return EXIT_SUCCESS;
}