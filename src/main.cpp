
#include <CGAL/Polyhedron_3.h>
#include "JsonWriter.hpp"

#define DATA_PATH "D:\\SP\\geoCFD\\data"

int main(int argc, const char** argv)
{

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
	std::string filename = "/3dbag_v210908_fd2cee53_5907.json";
	std::cout << "current reading file is: " << DATA_PATH + filename << '\n';
	std::ifstream input(DATA_PATH + filename);
	json j;
	input >> j;
	input.close();


	//read certain building
	JsonHandler jhandle1;
	std::string building1_id = "NL.IMBAG.Pand.0503100000019695-0";
	jhandle1.read_certain_building(j, building1_id);
	jhandle1.message();

	JsonHandler jhandle2;
	std::string building2_id = "NL.IMBAG.Pand.0503100000018413-0"; // adjacent to building1
	jhandle2.read_certain_building(j, building2_id);
	jhandle2.message();

	// // test output
	//
	// std::cout << "vertices number: " << '\n';
	// for (const auto& so : jhandle.solids)
	//    for (const auto& se : so.shells)
	//       for (const auto& f : se.faces)
	//          for (const auto& r : f.rings) // for most cases, each face only contains one ring -> i.e. face [[0,1,2,3]] only has one ring
	//             {
	//                std::cout << "--------" << '\n';
	//                for (const auto& indice : r.indices)
	//                   std::cout << indice << '\n';
	//             }


	// build a vector to store the nef polyhedra(if built successfully)
	std::vector<Nef_polyhedron> Nefs;


	// build polyhedron test
	BuildPolyhedron::build_one_polyhedron(jhandle1, Nefs);
	BuildPolyhedron::build_one_polyhedron(jhandle2, Nefs);


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

	/* test extracting geometries of a nef: use a simple polyhedron */
	// Point_3 p( 1.0, 0.0, 0.0);
	// Point_3 q( 0.0, 1.0, 0.0);
	// Point_3 r( 0.0, 0.0, 1.0);
	// Point_3 s( 0.0, 0.0, 0.0);
	// Polyhedron P;
	// P.make_tetrahedron( p, q, r, s);
	// std::cout<<"testing polyhedron\n";
	// std::cout<<P;

	// Nef_polyhedron test_nef(P);
	// std::cout<<"build test nef"<<'\n';

	/* test extracting geometries of a nef: use a simple polyhedron */



	// extract geometries and store in shell_explorers
	std::vector<Shell_explorer> shell_explorers;
	NefProcessing::extract_nef_geometries(big_nef, shell_explorers);

	/* process indices for writing to json file */
	// first store all the vertices in a vector
	std::vector<Point_3> all_vertices; // contains repeated vertices - for all shells
	for (auto const& se : shell_explorers) {
		for (auto const& v : se.vertices) {
			all_vertices.push_back(v);
		}
	}

	// next store the face indexes(accumulated from 0)	
	unsigned long index_in_all_vertices = 0;
	for (auto& se : shell_explorers) {
		for (auto& face : se.faces) {
			for (auto& index : face) {
				index = index_in_all_vertices++;
			}
		}
	}
	// now we have the all_vertices and shell_explorers to write to cityjson -----------------------------

   // test cleaned_vertices and cleaned_faces
	NefProcessing::process_shells_for_cityjson(shell_explorers);
	// std::cout<<"test cleaned vertices and cleaned faces\n";
	// // prompt some info
	// std::cout << "after processing for cityjson: " << '\n';
	// std::cout << "shell explorers size: " << shell_explorers.size() << '\n';
	// std::cout << "info for each shell\n";
	// for (const auto &se : shell_explorers)
	// {
	//    std::cout << "vertices size of this shell: " << se.cleaned_vertices.size() << '\n';
	//    std::cout << "faces size of this shell: " << se.cleaned_faces.size() << '\n';
	//    std::cout << '\n';
	// }
	/* process indices for writing to json file */


	/* get the convex hull of the big_nef, use all_vertices of all shells */
	Polyhedron convex_polyhedron; // define polyhedron to hold convex hull
	Nef_polyhedron big_nef_convexhull;
	CGAL::convex_hull_3(all_vertices.begin(), all_vertices.end(), convex_polyhedron);
	if (convex_polyhedron.is_closed()) {
		std::cout << " build convex hull " << '\n';
		Nef_polyhedron convex_nef(convex_polyhedron);
		big_nef_convexhull = convex_nef;
	}
	// extract geometries of big_nef_convexhull
	std::vector<Shell_explorer> shell_explorers_bignef_convexhull;
	Nef_polyhedron::Volume_const_iterator current_volume_bignef_convexhull;
	CGAL_forall_volumes(current_volume_bignef_convexhull, big_nef_convexhull) { // use big_nef_convexhull to replace the big_nef
		Nef_polyhedron::Shell_entry_const_iterator current_shell_bignef_convexhull;
		CGAL_forall_shells_of(current_shell_bignef_convexhull, current_volume_bignef_convexhull) {
			Shell_explorer se;
			Nef_polyhedron::SFace_const_handle sface_in_shell(current_shell_bignef_convexhull);
			big_nef_convexhull.visit_shell_objects(sface_in_shell, se); // use test_nef to replace the big_nef

			//add the se to shell_explorers_bignef_convexhull
			shell_explorers_bignef_convexhull.push_back(se);
		}
	}
	std::cout << "after extracting geometries of big nef convexhull: " << '\n';
	std::cout << "shell explorers big nef ocnvex hull size: " << shell_explorers.size() << '\n';
	std::cout << "info for each shell\n";
	for (const auto& se : shell_explorers_bignef_convexhull) {
		std::cout << "vertices size of this shell: " << se.vertices.size() << '\n';
		std::cout << "faces size of this shell: " << se.faces.size() << '\n';
		std::cout << '\n';
	}

	/* process indices for writing to json file */
	// first store all the vertices in a vector
	std::vector<Point_3> all_vertices_convex; // contains repeated vertices - for all shells
	for (auto const& se : shell_explorers_bignef_convexhull) {
		for (auto const& v : se.vertices) {
			all_vertices_convex.push_back(v);
		}
	}

	// next store the face indexes(accumulated from 0)	
	unsigned long index_in_all_vertices_convex = 0;
	for (auto& se : shell_explorers_bignef_convexhull) {
		for (auto& face : se.faces) {
			for (auto& index : face) {
				index = index_in_all_vertices_convex++;
			}
		}
	}
	// now we have the all_vertices and shell_explorers to write to cityjson -----------------------------
   /* process indices for writing to json file */


   /* test geometries for test_nef */
   // std::cout<<"output geometries of cleaned shell[0]"<<'\n';
   // const auto& test_se = shell_explorers[0];
   // std::cout<<"cleaned vertices size: "<<test_se.cleaned_vertices.size()<<'\n';
   // for(const auto& face : test_se.cleaned_faces)
   //    for(const auto& index : face)
   //       std::cout<<index<<" ";
   //    std::cout<<'\n';

   /* the problem is located in extracting geometries */


   /* get a test shell and test if there's any problem in outputing function */
   // Shell_explorer my_se;
   // my_se.vertices.emplace_back(Point_3(0,0,0));
   // my_se.vertices.emplace_back(Point_3(0,0,1));
   // my_se.vertices.emplace_back(Point_3(0,1,0));
   // my_se.vertices.emplace_back(Point_3(1,0,0));

   // my_se.faces.emplace_back();
   // my_se.faces.back() = {3,2,1};
   // my_se.faces.emplace_back();
   // my_se.faces.back() = {0,3,1};
   // my_se.faces.emplace_back();
   // my_se.faces.back() = {0,2,3};
   // my_se.faces.emplace_back();
   // my_se.faces.back() = {0,1,2};
   /* get a test shell and test if there's any problem in outputing function */

   // write file
	JsonWriter jwrite;
	std::string writeFilename = "/bignefpolyhedron.json";
	const Shell_explorer& shell = shell_explorers[1]; // which shell is going to be written to the file
	jwrite.write_json_file(DATA_PATH + writeFilename, shell);
	//jwrite.write_json_file(DATA_PATH + writeFilename, all_vertices, shell);

	return 0;
}