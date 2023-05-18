#pragma once

#include <chrono>
#include <thread>

namespace kko{

	void sleep(int ms){
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
	
}
