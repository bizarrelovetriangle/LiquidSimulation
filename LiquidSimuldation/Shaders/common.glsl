struct Particle {
	vec2 position;
	vec2 velosity;
	vec2 applied_impulse;
	vec2 acceleration;

	ivec2 grid_position;
	float radius;
	float density;
	float density_near;
	int index;
	bool sticky;
};

struct GridCell {
	int start;
	int end;
};

struct PairData {
	int first;
	int second;
	vec2 normal;
	float proximity_coefficient;
	float rest_length;
	bool torn;
};

struct Config {
	float interactionRange;
	float restDensity;
	float k;
	float k_near;
	float kLinearViscocity;
	float kQuadraticViscocity;
};

void swap(inout int v1, inout int v2) {
	int temp = v1;
	v1 = v2;
	v2 = temp;
}

struct Range {
	ivec2 a;
	ivec2 b;
};

Range GetRange(ivec2 grid_size, ivec2 grid_pos) {
	Range range;
	range.a = grid_pos - ivec2(1, 1);
	range.b = grid_pos + ivec2(1, 1);
	range.a.x = clamp(range.a.x, 0, grid_size.x - 1);
	range.a.y = clamp(range.a.y, 0, grid_size.y - 1);
	range.b.x = clamp(range.b.x, 0, grid_size.x - 1);
	range.b.y = clamp(range.b.y, 0, grid_size.y - 1);
	return range;
}