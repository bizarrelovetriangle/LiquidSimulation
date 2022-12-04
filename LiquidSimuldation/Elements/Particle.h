#pragma once
#include <Math/vector2.h>
#include <Math/matrix3x3.h>
#include <OpenGL/DeviceProgram/RenderProgram.h>
#include <Utils/DataFactory.h>
#include <Elements/ElementSharedData.h>

class Particle {
public:
	Particle(vector2 position, float radius = 2)
		: position(position), radius(radius)
	{
		_shared_data = DataFactory<ElementSharedData<Particle>>::GetData([]() {
			auto shared_data = std::make_shared<ElementSharedData<Particle>>();
			shared_data->initial_points = { {-1., -1.}, {1., -1.}, {-1., 1.}, {1., 1.} };
			shared_data->indexes = { 0, 1, 3, 0, 3, 2 };
			shared_data->render_program.InitProgram({
				{ GL_VERTEX_SHADER, "shaders/render/particle.vert" },
				{ GL_FRAGMENT_SHADER, "shaders/render/particle.frag" } });
			shared_data->render_program.GenerateVAO(shared_data->indexes);
			shared_data->render_program.Use();
			auto view_matrix = DataFactory<matrix3x3>::GetData();
			shared_data->render_program.SetViewMatrix(*view_matrix);
			shared_data->render_program.SetColor(vector3(0.5, 1., 1.));
			shared_data->render_program.UpdateVerteces(shared_data->initial_points);
			return shared_data;
		});

		static size_t _index = 0;
		index = _index++;
	}

	Particle() {}

	void Update(double interval) {
		velosity += acceleration * interval;
		position += velosity * interval;
	}

	sf::Vector2i gridPosition;

	vector2 position;
	vector2 velosity;
	vector2 acceleration;
	float radius;

	float density = 0;
	float density_near = 0;

	int index = 0;

private:
	std::shared_ptr<ElementSharedData<Particle>> _shared_data;
};