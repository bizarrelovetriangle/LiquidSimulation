#include "DeviceBuffer.h"
#include <glad/glad.h>
#include <DeviceComputation/PairCreator.h>
//#include <GLFW/glfw3.h>

template <typename T>
DeviceBuffer<T>::DeviceBuffer(size_t size)
{
	glCreateBuffers(1, &_buffer_id);
	_data.resize(size);
	glNamedBufferData(_buffer_id, _data.size() * sizeof(T), &_data[0], GL_DYNAMIC_DRAW);
}

template <typename T>
uint32_t& DeviceBuffer<T>::GetBufferId()
{
	return _buffer_id;
}

template<typename T>
std::vector<T>& DeviceBuffer<T>::GetData()
{
	return _data;
}

template<typename T>
void DeviceBuffer<T>::RetriveData()
{
	glGetNamedBufferSubData(_buffer_id, 0, _data.size() * sizeof(T), &_data[0]);
}

template<typename T>
void DeviceBuffer<T>::FlushData()
{
	glNamedBufferData(_buffer_id, _data.size() * sizeof(T), &_data[0], GL_DYNAMIC_DRAW);
	//glNamedBufferSubData(_buffer_id, 0, _data.size() * sizeof(T), &_data[0]);
}

template class DeviceBuffer<PairData>;
template class DeviceBuffer<int>;
