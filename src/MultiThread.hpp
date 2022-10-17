#pragma once

#include <vector>
#include <functional> // for std::reference_wrapper<T>
#include <future> // for std::async
#include <mutex> // for std::mutex
#include <chrono> // for Timer
#include <thread> // for std::this_thread::sleep_for(seconds(5));

#include "Polyhedron.hpp"
#include "JsonHandler.hpp"


using namespace std::chrono;


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


namespace MT {

	std::vector<Nef_polyhedron*> m_nef_ptrs; // hold the nefs
	std::vector<std::future<void>> m_futures; // store the return value of std::async, necessary step to make async work
	std::mutex s_nefs_mutex; // for thread-safety


    Nef_polyhedron* build_nef(const JsonHandler* const jtr)
    {
        //std::this_thread::sleep_for(seconds(5));

        // operations ... get polyhedron, get nef, get minkowski nef
        Nef_polyhedron* nef_ptr = Build::build_nef_polyhedron(*jtr);
        if (nef_ptr == nullptr) {
            std::cerr << "nef polyhedron not built, please check build_nef_polyhedron() function" << std::endl;
            return nullptr;
        }

        return nef_ptr; // further optimization: using pointers
    }
    
    
    
    void get_nefs(const JsonHandler* const jtr) {

        Nef_polyhedron* nef_ptr = build_nef(jtr);
        if (nef_ptr == nullptr) {
            std::cerr << "pointer allocation not succeed, please check build_nef() function" << std::endl;
            return;
        }

        // perform minkowski operation
        Nef_polyhedron merged_nef = NefProcessing::minkowski_sum(*nef_ptr);

        Nef_polyhedron* merged_nef_ptr = new(std::nothrow) Nef_polyhedron(merged_nef);
        if (merged_nef_ptr == nullptr) {
            std::cerr << "pointer allocation not succeed, please check build_nef() function" << std::endl;
            return;
        }

        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lock(s_nefs_mutex); // lock the meshes to avoid conflict
        m_nef_ptrs.emplace_back(merged_nef_ptr);

        delete nef_ptr;
        nef_ptr = nullptr;

    }



    // in this function we call std::async() method
    void get_nefs_async(const std::vector<JsonHandler>& jhandles) {
        
        std::cout << "size of buildings: " << jhandles.size() << std::endl;

        /*
        * it is important to save the result of std::async()
        * to enable the async process
        */
        for (const auto& jhandle : jhandles) {
            m_futures.emplace_back(std::async(std::launch::async, get_nefs, &jhandle));
            //get_nefs(&jhandle);
        }


        /*
        * if we wish to get the result value and keep processing
        * we need to use get() of every future object
        */
        for (auto& futureObject : m_futures) {
            futureObject.get();
        }
    }


    void clean() {
        for (unsigned int i = 0; i != m_nef_ptrs.size(); ++i) {
            delete m_nef_ptrs[i];
            m_nef_ptrs[i] = nullptr;
        }
    }


};