struct Particle {
	ivec2 gridPosition;

	vec2 position;
	vec2 velosity;
	vec2 acceleration;
	float radius;

	float density;
	float density_near;

	int index;

	vec4 _shared_data;
};

struct GridCell {
	int particles_start;
	int particles_end;

	int pairs_start;
	int pairs_end;
};

struct PairData {
	int first;
	int second;
	vec2 normal;
	float proximityCoefficient;
};

struct Config {
	float interactionRange;
	float restDensity;
	float k;
	float k_near;
	float kLinearViscocity;
	float kQuadraticViscocity;
};
