#pragma once
#include <string>
#include <unordered_map>
#include <chrono>

class NeatTimer
{
public:
	static NeatTimer& GetInstance();
	void StageBegin(const std::string& stage);
	void Refresh(std::chrono::seconds rate);
private:
	std::chrono::steady_clock clock_;
	std::string currentStage_;
	std::chrono::steady_clock::time_point lastTime_;
	std::unordered_map<std::string, std::chrono::steady_clock::time_point> stageMap_;
};

