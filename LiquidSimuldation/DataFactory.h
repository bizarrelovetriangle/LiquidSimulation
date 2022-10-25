#pragma once
#include <memory>
#include <mutex>

template <typename T>
class DataFactory {
public:
	template <typename F>
	static std::shared_ptr<T> GetData(F factoryFun) {
		std::lock_guard<std::mutex> lg(m);
		if (!_data) _data = factoryFun();
		return _data;
	}

	static std::shared_ptr<T> GetData() {
		if (!_data) throw new std::exception("data doesn't exist");
		return _data;
	}

private:
	inline static std::mutex m;
	inline static std::shared_ptr<T> _data = nullptr;
};
