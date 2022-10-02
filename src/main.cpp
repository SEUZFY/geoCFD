/*
* process geometry for cfd simulation
* main.cpp
*/


#include <CGAL/Polyhedron_3.h>
#include "JsonWriter.hpp"

#include<chrono> //Timer


#define DATA_PATH "D:\\SP\\geoCFD\\data" // specify the data path
//#define _ENABLE_CONVEX_HULL_ // switch on/off convex hull method
#define _ENABLE_MINKOWSKI_SUM_ // switch on/off minkowski sum method -> active by default



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



int main(int argc, const char** argv)
{
	Timer timer; // count the run time
	
	std::cout << "-- activated data folder: " << DATA_PATH << '\n';
	std::cout << "This is: " << argv[0] << '\n';

	//  std::cout<<"newly-added\n";
	//std::cout<<"data path is: "<<mypath<<'\n';

	//  char buffer[256];
	//  if (getcwd(buffer, sizeof(buffer)) != NULL) {
	//     printf("Current working directory : %s\n", buffer);
	//  } else {
	//     perror("getcwd() error");
	//     return 1;
	//  }

	//-- reading the (original)file with nlohmann json: https://github.com/nlohmann/json  
	std::string filename = "\\3dbag_v210908_fd2cee53_5907.json";
	std::cout << "current reading file is: " << DATA_PATH + filename << '\n';
	std::ifstream input(DATA_PATH + filename);
	json j;
	input >> j;
	input.close();

	double lod = 2.2; // specify the lod level

	// get ids of adjacent buildings
	const char* adjacency[] = { "NL.IMBAG.Pand.0503100000019695-0",
								"NL.IMBAG.Pand.0503100000018413-0",
								"NL.IMBAG.Pand.0503100000018423-0",
								"NL.IMBAG.Pand.0503100000018419-0",
								"NL.IMBAG.Pand.0503100000018408-0",
								"NL.IMBAG.Pand.0503100000018412-0",
								"NL.IMBAG.Pand.0503100000018407-0",
								"NL.IMBAG.Pand.0503100000018411-0", 
								"NL.IMBAG.Pand.0503100000018425-0",
								"NL.IMBAG.Pand.0503100000018422-0",
								"NL.IMBAG.Pand.0503100000018427-0",
								"NL.IMBAG.Pand.0503100000018409-0",
								"NL.IMBAG.Pand.0503100000004564-0",
								"NL.IMBAG.Pand.0503100000032517-0",
								"NL.IMBAG.Pand.0503100000019797-0",
								"NL.IMBAG.Pand.0503100000019796-0",
								"NL.IMBAG.Pand.0503100000004566-0",
								"NL.IMBAG.Pand.0503100000004565-0",
								"NL.IMBAG.Pand.0503100000031928-0",
								"NL.IMBAG.Pand.0503100000017031-0",
								"NL.IMBAG.Pand.0503100000027802-0",
								"NL.IMBAG.Pand.0503100000027801-0",
								"NL.IMBAG.Pand.0503100000018586-0" };


	//read certain building, stores in jhandlers vector
	std::vector<JsonHandler> jhandlers;
	//jhandlers.reserve(size); -> if we know the length of adjacency lsit, we can use reserve()
	std::cout << "------------------------ building(part) info ------------------------\n";
	
	for (auto const& building_name : adjacency) // get each building
	{
		JsonHandler jhandler;
		jhandler.read_certain_building(j, building_name, lod); // read in the building
		jhandler.message();
		jhandlers.emplace_back(jhandler); // add to the jhandlers vector
	}

	std::cout << "---------------------------------------------------------------------\n";

	// build a vector to store the nef polyhedra(if built successfully)
	std::vector<Nef_polyhedron> Nefs;

	// build Nef_polyhedron and sotres in Nefs vector
	for (auto const& jhandler : jhandlers)
	{
		BuildPolyhedron::build_nef_polyhedron(jhandler, Nefs);
	}

	// prompt Nefs
	std::cout << "there are " << Nefs.size() << " Nef polyhedra now\n";


	// check if Nef is simple and convert it to polyhedron 3 for visualise
	// for(const auto& nef : Nefs)
	// {
	//    std::cout<<"is nef simple? "<<nef.is_simple()<<'\n';
	//    if(nef.is_simple())
	//    {
	//       Polyhedron p;
	//       nef.convert_to_Polyhedron(p);
	//       std::cout<<p;
	//    }
	// }


	// big Nef
	Nef_polyhedron big_nef;
	for (const auto& nef : Nefs)
		big_nef += nef;


	// check if big Nef is simple - simple: no internal rooms, not simple: multiple rooms?
	std::cout << "is bigNef simple? " << big_nef.is_simple() << '\n';

	// process the big nef to make it available for output
	std::vector<Shell_explorer> shell_explorers; // store the extracted geometries
	NefProcessing::extract_nef_geometries(big_nef, shell_explorers); // extract geometries of the bignef
	NefProcessing::process_shells_for_cityjson(shell_explorers); // process shells for writing to cityjson

	// prompt some info after cleaning operation
	/*std::cout << "info about shells after cleaning operations\n";
	std::cout << "shell explorers size: " << shell_explorers.size() << '\n';
	std::cout << "info for each shell\n";
	for (const auto& se : shell_explorers)
	{
		std::cout << "cleaned vertices size of this shell: " << se.cleaned_vertices.size() << '\n';
		std::cout << "cleaned faces size of this shell: " << se.cleaned_faces.size() << '\n';
		std::cout << '\n';
	}*/


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


#ifdef _ENABLE_MINKOWSKI_SUM_
	// minkowski_sum_3 -> add a "buffer" for each nef in Nefs
	std::cout << "performing minkowski sum...\n";
	Nef_polyhedron merged_big_nef; 
	for (auto& nef : Nefs)
	{
		Nef_polyhedron merged_nef = NefProcessing::minkowski_sum(nef, 0.1); // cube size is 1.0 by default, can be altered
		merged_big_nef += merged_nef;
	}
	std::cout << "performing minkowski sum done\n";
	
	// process the merged big nef to make it available for output
	std::vector<Shell_explorer> merged_shell_explorers;
	NefProcessing::extract_nef_geometries(merged_big_nef, merged_shell_explorers);
	NefProcessing::process_shells_for_cityjson(merged_shell_explorers);
#endif


    // write file
	JsonWriter jwrite;
	std::string writeFilename = "\\buildingset_1_exterior_m=0.1.json";
	const Shell_explorer& shell = merged_shell_explorers[0]; // which shell is going to be written to the file, 0 - exterior, 1 - interior
	std::cout << "writing the result to cityjson file...\n";
	jwrite.write_json_file(DATA_PATH + writeFilename, shell, lod);

	return 0;
}