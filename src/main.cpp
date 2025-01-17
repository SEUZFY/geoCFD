﻿/*
* process geometry for cfd simulation
* main.cpp
*/



#include "JsonWriter.hpp"
#include "cmdline.h" // for cmd line parser
#include "MultiThread.hpp"



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

  p.add<std::string>("dataset", 'd', "dataset (.json)", true, ""); // dataset file
  p.add<std::string>("adjacency", 'a', "adjacency file (.txt)", true, ""); // adjacency file
  p.add<std::string>("path_result", 'p', "where the results will be saved", true, ""); // dataset file

  p.add<double>("lod", 'l', "lod level", false, 2.2, cmdline::oneof<double>(1.2, 1.3, 2.2)); // lod level, 2.2 by default
  p.add<double>("minkowski", 'm', "minkowski value", false, 0.01); // minkowski value, 0.01 by default
  p.add<double>("target edge length", 'e', "target edge length for remeshing", false, 3);

  p.add("remesh", '\0', "activate remeshing processing (warning: time consuming)");
  p.add("multi", '\0', "activate multi threading process"); // boolean flags
  p.add("json", '\0', "output as .json file format"); // boolean flags
  p.add("off", '\0', "output as .off file format"); // boolean flags
  p.add("all", '\0', "adjacency file contains all adjacent blocks"); // boolean flags
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
  std::string srcFile = p.get<std::string>("dataset");
  std::string adjacencyFile = p.get<std::string>("adjacency");
  std::string path = p.get<std::string>("path_result");
  double lod = p.get<double>("lod");
  double minkowski_param = p.get<double>("minkowski");
  double target_edge_length = p.get<double>("target edge length");
  bool enable_remeshing = p.exist("remesh");
  bool enable_multi_threading = p.exist("multi");
  bool all_adjacency_tag = p.exist("all");

  // pre-defined parameters
  //std::string srcFile = "D:\\SP\\geoCFD\\data\\3dbag_v210908_fd2cee53_5907.json";
  //std::string path = "D:\\SP\\geoCFD\\data";
  std::string delimiter = "\\";

  // optional parameters
  unsigned int adjacency_size = 50; /* number of adjacent buildings in one block */
  unsigned int adjacencies_size = 100; /* number of adjacencies in one tile */
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
  std::cout << "=> source file\t\t\t " << srcFile << '\n';
  std::cout << "=> adjacency\t\t\t " << adjacencyFile << '\n';
  std::cout << "=> all adjacency tag\t\t " << (all_adjacency_tag ? "true" : "false") << '\n';
  std::cout << "=> lod level\t\t\t " << lod << '\n';
  std::cout << "=> minkowksi parameter\t\t " << minkowski_param << '\n';
  std::cout << "=> enable remeshing\t\t " << (enable_remeshing ? "true" : "false") << '\n';
  std::cout << "=> target edge length\t\t " << target_edge_length << '\n';
  std::cout << "=> enable multi threading\t " << emt_string << '\n';
  std::cout << "=> output file folder\t\t " << path << '\n';
  std::cout << "=> output file format\t\t " << output_format << '\n';
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






  /* read in source file and shift the coordinates ------------------------------------------------------------------------*/
  std::ifstream input(srcFile);
  if (!input.is_open()) {
	std::cerr << "Error: Unable to open cityjson file \"" << srcFile << "\" for reading!" << std::endl;
	return 1;
  }
  json j;
  input >> j;
  input.close();

  // shift the coordinates
  // to maintain the adjacency property after shifting, the shifting process will be done for one tile
  std::tuple<double, double, double> datum = JsonHandler::get_translation_datum(j, lod);
  /* ----------------------------------------------------------------------------------------------------------------------*/






  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  * if all_adjacency_tag is marked as false, that means the input adjacency file only contains one block
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */





  /* one block -----------------------------------------------------------------------------------------------------------*/
  if (!all_adjacency_tag){

	// get ids of adjacent buildings
	std::vector<std::string> adjacency;
	adjacency.reserve(adjacency_size);
	FileIO::read_adjacency_from_txt(adjacencyFile, adjacency);

	// read buildings
	std::vector<JsonHandler> jhandles;
	jhandles.reserve(adjacency_size); // use reserve() to avoid extra copies

	// get jhandles, one jhandle for each building
	if (print_building_info)std::cout << "------------------------ building(part) info ------------------------\n";
	for (const auto& building_name : adjacency) // get each building
	{
	  JsonHandler jhandle;
	  jhandle.read_certain_building(j, building_name, lod, datum); // read in the building
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

	// remeshing
	if (enable_remeshing) {
	  std::cout << "remeshing ...\n";
	  std::string file = "remeshed.off";
	  PostProcesssing::remeshing(big_nef, path + delimiter + file, target_edge_length);
	  std::cout << "done\n";
	}

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

  } // end if: all_adjacency_tag
  /* ----------------------------------------------------------------------------------------------------------------------*/






  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  * if all_adjacency_tag is marked as true, that means the input adjacency file contains multiple blocks
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */





  if (all_adjacency_tag) {

	enable_multi_threading = false;
	std::cout << "multi threading process should not be enabled with all adjacencies\n";

	// declarations for convenient use
	using std::vector;
	using std::string;

	// store multiple adjacencies
	vector<vector<string>> adjacencies;
	adjacencies.reserve(adjacencies_size);
	FileIO::read_all_adjacencies_from_txt(adjacencyFile, adjacencies);


	// for each adjacency in adjacencies, we perform akin operations as above
	// some optimization can be done (e.g. via lambda function to avoid code repeatness) if having time


	// for mark the output files
	unsigned int num_off = 1;
	unsigned int num_json = 1;

	// for tracking which adjacency is currently being processed
	unsigned int num_adjacency = 1;

	// for storing constructed big_nefs
	vector<Nef_polyhedron> big_nefs;

	// needed vectors
	// after the usage for each adjacency, call clear() method for next use
	// avoid repeated creation whenever possible
	vector<JsonHandler> jhandles;  // hold jhandles, one jhandle for one building
	vector<Nef_polyhedron> nefs; // hold the nefs
	vector<Nef_polyhedron> expanded_nefs; // hold expanded nefs
	vector<Shell_explorer> shell_explorers; // hold shells for big nef

	jhandles.reserve(adjacency_size); // avoid reallocation, use reserve() whenever possible
	nefs.reserve(adjacency_size);
	expanded_nefs.reserve(adjacency_size);

	// process each adjacency
	for (const auto& adjacency : adjacencies) {

	  // track the adjacency - 1-based index, e.g. adjacency 1, adjacency 2, ...
	  std::cout << '\n';
	  std::cout << "processing adjacency " << num_adjacency << " ...\n";


	  // create big nef
	  // ------------------------------------------------------------------------------------------------------------------
	  // read buildings, get jhandles, one jhandle for each building
	  if (print_building_info)std::cout << "------------------------ building(part) info ------------------------\n";
	  for (const auto& building_name : adjacency) // get each building
	  {
		JsonHandler jhandle;
		jhandle.read_certain_building(j, building_name, lod, datum); // read in the building
		jhandles.emplace_back(jhandle); // add to the jhandlers vector

		if (print_building_info) {
		  jhandle.message();
		}
	  }
	  if (print_building_info)std::cout << "---------------------------------------------------------------------\n";


	  /* begin counting */
	  Timer timer; // count the run time


	  /* build the nef and stored in nefs vector */
	  for (const auto& jhdl : jhandles) {
		Build::build_nef_polyhedron(jhdl, nefs); // triangulation tag can be passed as parameters, set to true by default
	  }std::cout << "there are " << nefs.size() << " " << "nef polyhedra in total" << '\n';


	  /* perform minkowski sum operation and store expanded nefs in nefs_expanded vector */
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
	  big_nefs.emplace_back(big_nef);
	  // ------------------------------------------------------------------------------------------------------------------



	  // ------------------------------------------------------------------------------------------------------------------



	  std::cout << "adjacency " << num_adjacency << " done\n";
	  std::cout << '\n';
	  ++num_adjacency;



	  // vector cleaning for next use -------------------------------------------------------------------------------------
	  jhandles.clear();  // hold jhandles, one jhandle for one building
	  nefs.clear(); // hold the nefs
	  expanded_nefs.clear(); // hold expanded nefs
	  shell_explorers.clear(); // hold shells for big nef
	  // ------------------------------------------------------------------------------------------------------------------
	} // end for: adjacencies



	// --------------------------------------------------------------------------------------------------------------------
	std::cout << "adding all big nefs ...\n";
	Nef_polyhedron big_nef_all;
	for (auto& bignef : big_nefs) {
	  big_nef_all += bignef;
	}std::cout<< "done\n";


	// extract geometries and possible post-processing
	// extracting geometries
	NefProcessing::extract_nef_geometries(big_nef_all, shell_explorers); // extract geometries of the bignef
	NefProcessing::process_shells_for_cityjson(shell_explorers); // process shells for writing to cityjson


	// remeshing
	if (enable_remeshing) {
	  std::cout << "remeshing ...\n";
	  std::string file = "remeshed.off";
	  PostProcesssing::remeshing(big_nef_all, path + delimiter + file, target_edge_length);
	  std::cout << "done\n";
	}
	// ------------------------------------------------------------------------------------------------------------------



	// output
	// ------------------------------------------------------------------------------------------------------------------
	// write file
	// json
	if (OUTPUT_JSON) {

	  // get lod string
	  std::string lod_string;
	  if (std::abs(lod - 1.2) < epsilon)lod_string = "1.2";
	  if (std::abs(lod - 1.3) < epsilon)lod_string = "1.3";
	  if (std::abs(lod - 2.2) < epsilon)lod_string = "2.2";

	  // get minkowski param string
	  std::string mink_str = std::to_string(minkowski_param).substr(0, 5);

	  // get the sequence of the file
	  std::string num_str = std::to_string(num_json);

	  // output
	  std::string writeFilename = "lod=" + lod_string + "_" + "m=" + mink_str + "_" + num_str + ".json";
	  const Shell_explorer& shell = shell_explorers[1]; // which shell is going to be written to the file, 0 - exterior, 1 - interior
	  std::cout << "writing the result to cityjson file...\n";
	  FileIO::write_JSON(path + delimiter + writeFilename, shell, lod);

	  ++num_json; // increment the file sequence
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
	  std::string mink_str = std::to_string(minkowski_param).substr(0, 5);

	  // get the sequence of the file
	  std::string num_str = std::to_string(num_off);

	  // output
	  std::string writeFilename = "lod=" + lod_string + "_" + "m=" + mink_str + "_" + num_str + ".off";
	  std::cout << "writing the result to OFF file...\n";
	  bool status = FileIO::write_OFF(path + delimiter + writeFilename, big_nef_all);
	  if (!status) {
		std::cerr << "can not write .off file, please check" << '\n';
		return 1;
	  }

	  ++num_off; // increment the file sequence

	}


	// after processing all adjacencies, exit
	return EXIT_SUCCESS;

  }


  //std::cout << "no process performed, exit" << '\n';
  //return EXIT_SUCCESS;

}