#include <thread>

#include <gtest/gtest.h>

#include "spsc_queue.hpp"

TEST(SPSCQueue, exceptionForSizeNotPowerOfTwo) {
    EXPECT_THROW(SPSCQueue<std::size_t>(24), std::invalid_argument);
}

TEST(SPSCQueue, singleProducerAndConsumer) {
    const auto queue_size = 16;
    SPSCQueue<std::size_t> queue(queue_size);

    bool producer_is_done = false;

    // Write number 0 to 99 in the queue
    auto produce = std::jthread([&]() {
        for (std::size_t number{0}; number < 100; ++number) {
            while (!queue.push(number)) {
            }
        };

        producer_is_done = true;
    });


    std::vector<size_t> number_in_consumer;
    auto consumer = std::jthread([&]() {
        std::size_t current_item{10};
        auto has_values = true;

        while (!producer_is_done || has_values) {
            has_values = queue.pop(current_item);
            if (has_values) {
                number_in_consumer.push_back(current_item);
            }
        }
    });

    produce.join();
    consumer.join();

    EXPECT_EQ(number_in_consumer.size(), 100);
    for (std::size_t index{0}; index < number_in_consumer.size(); ++index) {
        EXPECT_EQ(number_in_consumer[index], index);
    }
}
