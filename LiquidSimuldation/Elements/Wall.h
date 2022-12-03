#pragma once
#include <SFML/Graphics.hpp>
#include <Utils/DataFactory.h>
#include <Elements/ElementSharedData.h>

class Wall {
public:
	Wall(sf::Vector2f a, sf::Vector2f b)
		: a(a), b(b)
	{
		_shared_data = DataFactory<ElementSharedData<Wall>>::GetData([&]() {
			auto shared_data = std::make_shared<ElementSharedData<Wall>>();
			shared_data->indexes = { 0, 1 };
			shared_data->render_program.InitProgram({
				{ GL_VERTEX_SHADER, "shaders/render/wall.vert" },
				{ GL_FRAGMENT_SHADER, "shaders/render/wall.frag" } });
			shared_data->render_program.GenerateVAO(shared_data->indexes);
			shared_data->render_program.Use();
			auto view_matrix = DataFactory<matrix3x3>::GetData();
			shared_data->render_program.SetViewMatrix(*view_matrix);
			shared_data->render_program.SetColor(vector3(0.5, 0.5, 1.));
			return shared_data;
		});

		_initial_points = { a, b };
	}

	void draw() {
		_shared_data->render_program.UpdateVerteces(_initial_points);
		_shared_data->render_program.Use();
		glDrawElements(GL_LINES, _shared_data->indexes.size(), GL_UNSIGNED_INT, 0);
	}

	sf::Vector2f a, b;

private:
	std::vector<vector2> _initial_points;
	std::shared_ptr<ElementSharedData<Wall>> _shared_data;
};