#include "DeviceBuffer.h"
#include <glad/glad.h>
#include <DeviceComputation/PairCreator.h>
#include <Config.h>
#include <ParticleGrid.h>
#include <Elements/Particle.h>

template <typename T>
DeviceBuffer<T>::DeviceBuffer(size_t size) : _size(size)
{
	glCreateBuffers(1, &_buffer_id);
	glNamedBufferData(_buffer_id, _size * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
}

template <typename T>
uint32_t& DeviceBuffer<T>::GetBufferId()
{
	return _buffer_id;
}

template<typename T>
std::vector<T> DeviceBuffer<T>::Retrive()
{
	std::vector<T> data(_size);
	glGetNamedBufferSubData(_buffer_id, 0, data.size() * sizeof(T), &data[0]);
	return data;
}

template<typename T>
std::vector<T> DeviceBuffer<T>::Retrive(size_t size)
{
	std::vector<T> data(size);
	glGetNamedBufferSubData(_buffer_id, 0, size * sizeof(T), &data[0]);
	return data;
}

template<typename T>
void DeviceBuffer<T>::Clear()
{
	uint32_t zero = 0;
	glClearNamedBufferData(_buffer_id, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &zero);
}

template<typename T>
void DeviceBuffer<T>::Resize(size_t size)
{
	_size = size;
	glNamedBufferData(_buffer_id, _size * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
}

template<typename T>
void DeviceBuffer<T>::Flush(const std::vector<T>& data)
{
	_size = data.size();
	glNamedBufferData(_buffer_id, data.size() * sizeof(T), &data[0], GL_DYNAMIC_DRAW);
}

template class DeviceBuffer<PairData>;
template class DeviceBuffer<int>;
template class DeviceBuffer<Config>;
template class DeviceBuffer<Particle>;
template class DeviceBuffer<ParticleGrid::GridCell>;
