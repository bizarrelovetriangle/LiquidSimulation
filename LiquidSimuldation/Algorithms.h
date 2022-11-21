#pragma once;

namespace Algorithms {
	template <typename F, typename T>
	void RadixSort(std::vector<T>& objects, F get_hashed) {
		constexpr size_t chunk_size = 8;
		constexpr size_t buckets_size = 1 << chunk_size;
		constexpr size_t mask = buckets_size - 1;

		std::vector<T> temp(objects.size());

		for (size_t chunk_index = 0; chunk_index < sizeof(T); chunk_index += chunk_size) {
			std::array<size_t, buckets_size> buckets{};
			std::array<size_t, buckets_size> bucket_indexes{};

			for (auto& obj : objects) {
				int hash = get_hashed(obj) >> chunk_index & mask;
				buckets[hash]++;
			}

			for (size_t i = 1; i < buckets.size(); ++i) {
				bucket_indexes[i] = bucket_indexes[i - 1] + buckets[i - 1];
			}

			for (auto& obj : objects) {
				int hash = get_hashed(obj) >> chunk_index & mask;
				size_t index = bucket_indexes[hash]++;
				temp[index] = obj;
			}

			std::swap(objects, temp);
		}
	}
}