/*
* process geometry for cfd simulation
* main.cpp
*/

#include "JsonWriter.hpp"
#include "cmdline.h" // for cmd line parser
#include "MultiThread.hpp"



//#define _ENABLE_CONVEX_HULL_ // switch on/off convex hull method



// entry point
int main(int argc, char* argv[])
{
	/* use parsers to add options -------------------------------------------------------------------------------------------*/
	

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* 
	* 1st argument is long name
	* 2nd argument is short name (no short name if '\0' specified)
	* 3rd argument is description
	* 4th argument is mandatory (optional. default is false)
	* 5th argument is default value  (optional. it is used when mandatory is false)
	* 6th argument is extra constraint.
	* 
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


	cmdline::parser p;

	p.add<std::string>("adjacency", 'a', "adjacency file (.txt)", true, ""); // adjacency file
	p.add<double>("lod", 'l', "lod level", false, 2.2, cmdline::oneof<double>(1.2, 1.3, 2.2)); // lod level, 2.2 by default
	p.add<double>("minkowski", 'm', "minkowski value", false, 0.01); // minkowski value, 0.01 by default
	p.add("multi", '\0', "activate multi threading process"); // boolean flags
	p.add("json", '\0', "output as .json file format"); // boolean flags
	p.add("off", '\0', "output as .off file format"); // boolean flags
	p.add("help", 0, "print this message"); // help option
	p.set_program_name("geocfd"); // set the program name in the console


	/* ----------------------------------------------------------------------------------------------------------------------*/
	


	/* run parser -----------------------------------------------------------------------------------------------------------*/
	bool ok = p.parse(argc, argv);

	if (argc == 1 || p.exist("help")) {
		std::cerr << p.usage();
		return 0;
	}

	if (!ok) {
		std::cerr << p.error() << std::endl << p.usage();
		return 0;
	}
	/* ----------------------------------------------------------------------------------------------------------------------*/
	


	/* get parameters -------------------------------------------------------------------------------------------------------*/
	std::string adjacencyFile = p.get<std::string>("adjacency");
	double lod = p.get<double>("lod");
	double minkowski_param = p.get<double>("minkowski");
	bool enable_multi_threading = p.exist("multi");

	// pre-defined parameters
	std::string srcFile = "D:\\SP\\geoCFD\\data\\3dbag_v210908_fd2cee53_5907.json";
	std::string path = "D:\\SP\\geoCFD\\data";
	std::string delimiter = "\\";

	// optional parameters
	unsigned int adjacency_size = 50; /* number of adjacent buildings in one block */
	bool print_building_info = false; /* whether to print the building info to the console */

	// output files
	bool OUTPUT_JSON = p.exist("json");
	bool OUTPUT_OFF = p.exist("off");
	std::string output_format;
	if (OUTPUT_JSON && OUTPUT_OFF == false) {
		output_format = ".json";
	}
	else if (OUTPUT_OFF && OUTPUT_JSON == false) {
		output_format = ".off";
	}
	else if (OUTPUT_JSON == false && OUTPUT_OFF == false) {
		OUTPUT_JSON = true;
		OUTPUT_OFF = true;
		output_format = ".json and .off"; // if no format specified, output two formats
	}
	else {
		output_format = ".json and .off";
	}
	/* ----------------------------------------------------------------------------------------------------------------------*/
	


	/* print the parameters -------------------------------------------------------------------------------------------------*/
	std::string emt_string = enable_multi_threading ? "true" : "false";
	std::cout << '\n';
	std::cout << "====== this is: " << argv[0] << " ======" << '\n';
	std::cout << "=> source file:            " << srcFile << '\n';
	std::cout << "=> adjacency:              " << adjacencyFile << '\n';
	std::cout << "=> lod level:              " << lod << '\n';
	std::cout << "=> minkowksi parameter:    " << minkowski_param << '\n';
	std::cout << "=> enable multi threading: " << emt_string << '\n';
	std::cout << "=> output file folder:     " << path << '\n';
	std::cout << "=> output file format:     " << output_format << '\n';
	std::cout << '\n';
	/* ----------------------------------------------------------------------------------------------------------------------*/
	


	/* ready to go? ---------------------------------------------------------------------------------------------------------*/
	std::cout << "Proceed ? [y/n]" << '\n';
	char proceed;
	std::cin >> proceed;
	if (proceed == 'n' || proceed == 'N') {
		std::cout << "Proceeding aborted" << '\n';
		return 0;
	}
	/* ----------------------------------------------------------------------------------------------------------------------*/



	std::ifstream input(srcFile);
	if (!input.is_open()) {
		std::cerr << "Error: Unable to open cityjson file \"" << srcFile << "\" for reading!" << std::endl;
		return 1;
	}
	json j;
	input >> j;
	input.close();

	// get ids of adjacent buildings	
	std::vector<std::string> adjacency;
	adjacency.reserve(adjacency_size);
	FileIO::read_adjacency_from_txt(adjacencyFile, adjacency);

	// read buildings
	std::vector<JsonHandler> jhandles;
	jhandles.reserve(adjacency_size); // use reserve() to avoid extra copies


	if (print_building_info)std::cout << "------------------------ building(part) info ------------------------\n";

	for (auto const& building_name : adjacency) // get each building
	{
		JsonHandler jhandle;
		jhandle.read_certain_building(j, building_name, lod); // read in the building
		jhandles.emplace_back(jhandle); // add to the jhandlers vector

		if (print_building_info) {
			jhandle.message();
		}
	}

	if (print_building_info)std::cout << "---------------------------------------------------------------------\n";



	/* begin counting */
	Timer timer; // count the run time



	/* build the nef and stored in nefs vector */
	std::vector<Nef_polyhedron> nefs; // hold the nefs
	nefs.reserve(adjacency_size); // avoid reallocation, use reserve() whenever possible
	for (const auto& jhdl : jhandles) {
		Build::build_nef_polyhedron(jhdl, nefs); // triangulation tag can be passed as parameters, set to true by default
	}std::cout << "there are " << nefs.size() << " " << "nef polyhedra in total" << '\n';



	/* perform minkowski sum operation and store expanded nefs in nefs_expanded vector */
	std::vector<Nef_polyhedron> expanded_nefs;
	expanded_nefs.reserve(adjacency_size); // avoid reallocation, use reserve() whenever possible



	/* performing minkowski operations -------------------------------------------------------------------------*/
	std::cout << "performing minkowski sum ... " << '\n';
	if (enable_multi_threading) {
		std::cout << "multi threading is enabled" << '\n';
		MT::expand_nefs_async(nefs, expanded_nefs, minkowski_param);
	}
	else {
		MT::expand_nefs(nefs, expanded_nefs, minkowski_param);
	}
	std::cout << "done" << '\n';
	/* building nefs and performing minkowski operations -------------------------------------------------------------------------*/



	// merging nefs into one big nef
	std::cout << "building big nef ..." << '\n';
	Nef_polyhedron big_nef;
	for (auto& nef : expanded_nefs) {
		big_nef += nef;
	}
	std::cout << "done" << '\n';



	// erosion ---------------------------------------------------------------------------------
	//std::cout << "processing for erosion ..." << '\n';
	//Nef_polyhedron eroded_big_nef = PostProcesssing::get_eroded_nef(big_nef, minkowski_param);
	//std::cout << "done" << '\n';
	// change big_nef to eroded_big_nef in the extract_nef_geometries() function
	// change big_nef to eroded_big_nef in the output functions
	// erosion ---------------------------------------------------------------------------------
	


	// extracting geometries
	std::vector<Shell_explorer> shell_explorers; // store the extracted geometries
	NefProcessing::extract_nef_geometries(big_nef, shell_explorers); // extract geometries of the bignef
	NefProcessing::process_shells_for_cityjson(shell_explorers); // process shells for writing to cityjson

	

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



    // write file
	// json
	if (OUTPUT_JSON) {

		// get lod string
		std::string lod_string;
		if (std::abs(lod - 1.2) < epsilon)lod_string = "1.2";
		if (std::abs(lod - 1.3) < epsilon)lod_string = "1.3";
		if (std::abs(lod - 2.2) < epsilon)lod_string = "2.2";

		// get minkowski param string
		std::string minkowski_string = std::to_string(minkowski_param);

		// output
		std::string writeFilename = "interior_lod=" + lod_string + "_" + "m=" + minkowski_string + ".json";
		const Shell_explorer& shell = shell_explorers[1]; // which shell is going to be written to the file, 0 - exterior, 1 - interior
		std::cout << "writing the result to cityjson file...\n";
		FileIO::write_JSON(path + delimiter + writeFilename, shell, lod);
	}
	
	// write file
	// OFF
	if (OUTPUT_OFF) {
		
		// get lod string
		std::string lod_string;
		if (std::abs(lod - 1.2) < epsilon)lod_string = "1.2";
		if (std::abs(lod - 1.3) < epsilon)lod_string = "1.3";
		if (std::abs(lod - 2.2) < epsilon)lod_string = "2.2";

		// get minkowski param string
		std::string minkowski_string = std::to_string(minkowski_param);

		// output
		std::string writeFilename = "exterior_lod=" + lod_string + "_" + "m=" + minkowski_string + ".off";
		std::cout << "writing the result to OFF file...\n";
		bool status = FileIO::write_OFF(path + delimiter + writeFilename, big_nef);
		if (!status) {
			std::cerr << "can not write .off file, please check" << '\n';
			return 1;
		}
	}

	return EXIT_SUCCESS;
}