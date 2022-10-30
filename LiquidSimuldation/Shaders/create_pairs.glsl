#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

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
    int start;
    int end;
};

struct PairData {
    int first;
    int second;
    vec2 normal;
    float proximityCoefficient;
};

layout(std430, binding = 0) buffer ParticlesInput
{
    Particle particles[];
};
layout(std430, binding = 1) buffer GridInput
{
    GridCell grid[];
};
layout(std430, binding = 2) buffer PairsOutput
{
    int pairs_count;
    PairData pairs[];
};

layout(location = 0) uniform int interactionRange;
layout(location = 1) uniform int gridColumns;
layout(location = 2) uniform int gridRows;

void main() {
	int id = int(gl_GlobalInvocationID.x);

	ivec2 range_a = particles[id].gridPosition - ivec2(1, 1);
	ivec2 range_b = particles[id].gridPosition + ivec2(1, 1);

	range_a.x = clamp(range_a.x, 0, gridColumns - 1);
	range_a.y = clamp(range_a.y, 0, gridRows - 1);
	range_b.x = clamp(range_b.x, 0, gridColumns - 1);
	range_b.y = clamp(range_b.y, 0, gridRows - 1);

	for (int y = range_a.y; y <= range_b.y; ++y) {
		for (int x = range_a.x; x <= range_b.x; ++x) {
			int grid_position = y * gridColumns + x;
			int start = grid[grid_position].start;
			int end = grid[grid_position].end;

			for (int j = start; j < end; ++j) {
				if (particles[id].index <= particles[j].index) continue;

				vec2 vector = particles[j].position - particles[id].position;
				float vectorLength = length(vector);

				if (vectorLength < interactionRange) {
					int pair_id = atomicAdd(pairs_count, 1);
					pairs[pair_id].first = id;
					pairs[pair_id].second = j;
					pairs[pair_id].normal = vector / vectorLength;
					pairs[pair_id].proximityCoefficient = 1 - vectorLength / interactionRange;
				}
			}
		}
	}
}