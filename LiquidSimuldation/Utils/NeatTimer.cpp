#include "NeatTimer.h"
#include <iostream>
#include <numeric>

NeatTimer& NeatTimer::GetInstance() {
	static NeatTimer nearTimer;
	return nearTimer;
}

void NeatTimer::StageBegin(const std::string& stage)
{
	if (currentStage_ == stage) return;
	auto now = clock_.now();

	if (!currentStage_.empty()) {
		auto duration = now - lastTime_;
		if (auto pair = _stage_map.try_emplace(std::move(currentStage_), duration); !pair.second) {
			pair.first->second += duration;
		}
	}

	currentStage_ = stage;
	lastTime_ = now;
}

void NeatTimer::Refresh(std::chrono::seconds rate)
{
	static size_t frames = 0;
	{
		static std::chrono::steady_clock::time_point lastRefresh;
		auto now = clock_.now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRefresh);
		++frames;
		if (rate > duration) return;
		lastRefresh = now;
	}

	size_t maxStageName = 0;
	std::chrono::milliseconds overallTime{};

	for (auto& pair : _stage_map) {
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(pair.second.time_since_epoch());
		overallTime += milliseconds;
		maxStageName = std::max(maxStageName, pair.first.length());
	}

	std::stringstream ss;
	ss << "Overall time consumption" << std::endl;

	for (auto& pair : _stage_map) {
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(pair.second.time_since_epoch());
		auto relativeTime = double(milliseconds.count()) / overallTime.count();
		auto offset = std::string(maxStageName - pair.first.length(), ' ');

		ss << "|" << pair.first << "| " << offset
			<< std::format("{:.5f}", relativeTime) << "%, " 
			<< milliseconds.count() << "ms"
			<< std::endl;

		pair.second = {};
	}

	ss << "fps - " << 1000. * frames / overallTime.count() << std::endl;
	frames = 0;

	std::cout << ss.str() << std::endl;
}
