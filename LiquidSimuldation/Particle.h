#pragma once
#include <SFML/Graphics.hpp>
#include "vector2.h"
#include "matrix3x3.h"
#include "RenderProgram.h"
#include "DataFactory.h"
#include "ElementSharedData.h"

class Particle {
public:
	Particle(vector2 position, float radius = 4)
		: position(position), radius(radius)
	{
		_shared_data = DataFactory<ElementSharedData<Particle>>::GetData([]() {
			auto shared_data = std::make_shared<ElementSharedData<Particle>>();
			shared_data->initial_points = { {-1., -1.}, {1., -1.}, {-1., 1.}, {1., 1.} };
			shared_data->indexes = { 0, 1, 3, 0, 3, 2 };
			shared_data->render_program.InitProgram({
				{ GL_VERTEX_SHADER, "Shaders/particle.vert" },
				{ GL_FRAGMENT_SHADER, "Shaders/particle.frag" } });
			shared_data->render_program.GenerateVAO(shared_data->indexes);
			shared_data->render_program.Use();
			auto view_matrix = DataFactory<matrix3x3>::GetData();
			shared_data->render_program.SetViewMatrix(*view_matrix);
			shared_data->render_program.SetColor(vector3(0.5, 1., 1.));
			return shared_data;
		});

		static size_t _index = 0;
		index = _index++;
	}

	void update(double interval) {
		velosity += acceleration * interval;
		position += velosity * interval;
	}

	void draw() {
		auto _model_matrix = matrix3x3();
		_model_matrix.scale(radius);
		_model_matrix.transfer(position);

		std::vector<vector2> points(_shared_data->initial_points.size());
		for (int i = 0; i < _shared_data->initial_points.size(); ++i) {
			points[i] = _model_matrix.multiply(_shared_data->initial_points[i], 1.);
		}

		_shared_data->render_program.UpdateVerteces(points);
		_shared_data->render_program.Use();
		glDrawElements(GL_TRIANGLES, _shared_data->indexes.size(), GL_UNSIGNED_INT, 0);
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