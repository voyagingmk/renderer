#ifndef RENDERER_PROFILER_HPP
#define RENDERER_PROFILER_HPP

#include <ctime>
#include <chrono>
#include <string>

namespace renderer {
	class Profiler {
		typedef std::chrono::steady_clock::time_point time_point;
		time_point time_begin;
		std::string flag;
	public:
		Profiler(std::string f);
		~Profiler();
	};
};

#endif

