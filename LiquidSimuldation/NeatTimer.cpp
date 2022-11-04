#include "NeatTimer.h"
#include <iostream>

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
		if (auto pair = stageMap_.try_emplace(std::move(currentStage_), duration); !pair.second) {
			pair.first->second += duration;
		}
	}

	currentStage_ = stage;
	lastTime_ = now;
}

void NeatTimer::Refresh(std::chrono::seconds rate)
{
	{
		static std::chrono::steady_clock::time_point lastRefresh;
		auto now = clock_.now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRefresh);
		if (rate > duration) return;
		lastRefresh = now;
	}

	size_t maxStageName = 0;
	std::chrono::milliseconds overallTime{};

	for (auto& pair : stageMap_) {
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(pair.second.time_since_epoch());
		overallTime += milliseconds;
		maxStageName = std::max(maxStageName, pair.first.length());
	}

	std::stringstream ss;
	ss << "Overall time consumption" << std::endl;

	for (auto& pair : stageMap_) {
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(pair.second.time_since_epoch());
		pair.second = {};
		auto relativeTime = double(milliseconds.count()) / overallTime.count();
		auto offset = std::string(maxStageName - pair.first.length(), ' ');

		ss << "|" << pair.first << "| " << offset
			<< std::format("{:.5f}", relativeTime) << "%, " 
			<< milliseconds.count() << "ms";
		ss << std::endl;
	}

	std::cout << ss.str() << std::endl;
}
