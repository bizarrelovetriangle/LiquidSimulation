#pragma once
#include <Utils/DataFactory.h>
#include <Elements/ElementSharedData.h>

class Wall {
public:
	Wall(vector2 a, vector2 b)
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
		center = (a + b) / 2;
	}

	void Update(float dt) {
		matrix3x3 rotate =
			matrix3x3::transfer(center) *
			matrix3x3::rotate(rotate_speed * dt) *
			matrix3x3::transfer(-center);

		for (auto& point : _initial_points) point = rotate * point;
	}

	void draw() {
		_shared_data->render_program.UpdateVerteces(_initial_points);
		_shared_data->render_program.Use();
		glDrawElements(GL_LINES, _shared_data->indexes.size(), GL_UNSIGNED_INT, 0);
	}

	const vector2& a() const { return _initial_points[0]; };
	const vector2& b() const { return _initial_points[1]; };

	float rotate_speed = 0;

private:
	std::vector<vector2> _initial_points;
	std::shared_ptr<ElementSharedData<Wall>> _shared_data;
	vector2 center;
};