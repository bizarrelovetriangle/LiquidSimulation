#include "Particle.h"

class ParticleGrid {
public:
	ParticleGrid(std::vector<Particle*> particles)
		: _particles(particles), _gridSize(30)
	{

	}

	void UpdateParticleNeightbours() {

	}

private:
	std::vector<Particle*> _particles;
	int _gridSize;
};