#include "stdafx.h"
#include "utils/profiler.hpp"


namespace renderer {

	Profiler::Profiler(std::string f) : flag(f) {
		time_begin = std::chrono::high_resolution_clock::now();
	}

	Profiler::~Profiler() {
		time_point time_end = std::chrono::high_resolution_clock::now();
		auto time_cost = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_begin).count();
		printf("[%s] time cost: %0.2f ms\n", flag.c_str(), time_cost*0.001);
	}
}