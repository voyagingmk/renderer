#include "stdafx.h"
#include "profiler.hpp"


namespace renderer {

	Profiler::Profiler(std::string f) : flag(f) {
		time_begin = std::chrono::system_clock::now();
	}

	Profiler::~Profiler() {
		time_point time_end = std::chrono::system_clock::now();
		auto time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count();
		printf("[%s] time cost: %lld ms\n", flag.c_str(), time_cost);
	}
}