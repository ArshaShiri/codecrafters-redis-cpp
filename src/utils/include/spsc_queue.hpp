#pragma once

#include <atomic>
#include <cstddef>
#include <new>
#include <vector>

/**
 * @brief A fixed size single producer, single consumer and thread safe queue which is lock free.
 * For performance reasons, the capacity should be a power of two.
 *
 * @tparam T Type of the objects stored in the queue.
 */
template<typename T>
class SPSCQueue {
  public:
    explicit SPSCQueue(std::size_t capacity) : capacity_{capacity}, store_(capacity * sizeof(T)) {
        const auto is_power_of_two = ((capacity) & (capacity - 1)) == 0;
        if (!is_power_of_two) {
            throw std::invalid_argument("Size must be a power of two");
        }
    }

    /**
     * @brief Push and item in the queue.
     *
     * @param item that is placed in the queue.
     * @return true when the item is pushed in the queue.
     * @return false when the queue is full.
     */
    bool push(T object) {
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        const auto next_tail = (current_tail + 1) & (capacity_ - 1);

        // Queue is full.
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }


        new (store_.data() + current_tail * sizeof(T)) T(std::move(object));
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pop an item into the given item.
     *
     * @param item where the popped item is stored.
     * @return true when item can be popped.
     * @return false when the queue is empty.
     */
    bool pop(T &item) {
        const auto current_head = head_.load(std::memory_order_relaxed);
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        auto top_ptr = reinterpret_cast<T *>(store_.data() + current_head * sizeof(T));
        item = std::move(*top_ptr);
        head_.store((current_head + 1) & (capacity_ - 1), std::memory_order_release);
        return true;
    }


  private:
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
    std::size_t capacity_{0};
    std::vector<std::byte> store_;
};
