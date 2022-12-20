#pragma once
#include <cstdint>
#include <vector>

template <typename T>
class DeviceBuffer
{
public:
	DeviceBuffer(size_t size = 0);
	uint32_t& GetBufferId();
	std::vector<T> Retrive();
	void Clear();
	void Resize(size_t size);
	void Flush(const std::vector<T>& data);
private:
	uint32_t _buffer_id = 0;
	size_t _size;
};
