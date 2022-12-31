#include "HolderTool.h"
#include "Scene.h"
#include <ranges>

HolderTool::HolderTool(Scene& scene)
	: Tool(scene)
{
}

void HolderTool::OnClick() {
	clicked = true;

	auto& particle_indexes = _scene._fluidProcessor->_particle_grid.particle_indexes;
	auto& particles =_scene._fluidProcessor->_particle_grid.particles;
	auto result = particle_indexes
		| std::views::filter([&](int index){ return (particles[index].position - _mouse_pos).length() < range; })
		| std::views::transform([&](int index){ return std::pair(index, particles[index].position - _mouse_pos); });
	_controlled_particles = { std::begin(result), std::end(result) };
}

void HolderTool::OnMoved(const vector2& pos) {
	_mouse_pos = pos;
	if (!clicked) return;

	auto& particle_indexes = _scene._fluidProcessor->_particle_grid.particle_indexes;
	auto& particles = _scene._fluidProcessor->_particle_grid.particles;

	for (auto& pair : _controlled_particles) {
		auto& particle = particles[pair.first];
		auto stretch = (_mouse_pos + pair.second) - particle.position;
		particle.velosity += (stretch - particle.velosity / 4) * _scene._expected_delta_time * 30;
	}
}

void HolderTool::OnRelease() {
	clicked = false;
	_controlled_particles.clear();
}

void HolderTool::OnKey(int key, int action) {

}
