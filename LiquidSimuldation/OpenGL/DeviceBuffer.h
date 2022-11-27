#pragma once
#include <cstdint>
#include <vector>

template <typename T>
class DeviceBuffer
{
public:
	DeviceBuffer(size_t size);
	uint32_t& GetBufferId();
	std::vector<T>& GetData();
	void RetriveData();
	void FlushData();
	void FlushData(const std::vector<T>& data);
private:
	uint32_t _buffer_id = 0;
	std::vector<T> _data;
};
