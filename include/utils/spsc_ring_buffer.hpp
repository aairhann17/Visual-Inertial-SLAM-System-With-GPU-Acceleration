#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <optional>

namespace vislam::utils {

namespace detail {

#if defined(__cpp_lib_hardware_interference_size) && !defined(__GNUC__)
constexpr std::size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
constexpr std::size_t kCacheLineSize = 64;
#endif

}  // namespace detail

template <typename T, std::size_t Capacity>
class SpscRingBuffer {
	static_assert(Capacity >= 2, "SpscRingBuffer requires Capacity >= 2.");

	public:
		bool Push(const T& value) {
			const auto head = head_.load(std::memory_order_relaxed);
			const auto next = Next(head);
			if (next == tail_.load(std::memory_order_acquire)) {
				return false;
			}

			buffer_[head] = value;
			head_.store(next, std::memory_order_release);
			return true;
		}

		std::optional<T> Pop() {
			const auto tail = tail_.load(std::memory_order_relaxed);
			if (tail == head_.load(std::memory_order_acquire)) {
				return std::nullopt;
			}

			T value = buffer_[tail];
			tail_.store(Next(tail), std::memory_order_release);
			return value;
		}

		[[nodiscard]] bool Empty() const {
			return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
		}

	private:
		[[nodiscard]] static constexpr std::size_t Next(const std::size_t index) {
			return (index + 1) % Capacity;
		}

		alignas(detail::kCacheLineSize) std::atomic<std::size_t> head_ {0};
		alignas(detail::kCacheLineSize) std::atomic<std::size_t> tail_ {0};
		std::array<T, Capacity> buffer_ {};
};

}  // namespace vislam::utils

