#pragma once;

class Config {
public:
	static Config& GetInstance() {
		static Config config;
		return config;
	}

	float interactionRange = 90;
	float restDensity = 50;
	float k = 15;
	float k_near = 800;
	float kLinearViscocity = 0.042;
	float kQuadraticViscocity = 0.05;
};