#pragma once
#include <vector>
#include <OpenGL/DeviceProgram/DeviceProgram.h>
#include <Math/vector2.h>
#include <Math/vector3.h>
#include <Math/matrix3x3.h>

template <typename T>
class RenderProgram : public DeviceProgram {
public:
	void Use();
	void GenerateVAO(const std::vector<uint32_t>& indexes);
	void UpdateVerteces(const std::vector<vector2>& points);
	void SetViewMatrix(const matrix3x3& matrix);
	void SetColor(const vector3& color);
	void SetIndex(int index);
	void Dispose();

private:
	uint32_t vao_buffer_id = 0;
	uint32_t vbo_buffer_id = 0;
	uint32_t ebo_buffer_id = 0;

	uint32_t view_matrix_index = 0;
	uint32_t color_index = 2;
	uint32_t index_index = 3;
};
