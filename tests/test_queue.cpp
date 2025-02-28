
#include <thread>   
#include <mutex>
#include <atomic>
#include <cmath>

#include <gtest/gtest.h>

#include <francos/queue.hpp>

using namespace francos;


TEST(TestQueue, SimplePushPop) {

    Queue<double, 5> queue;
    queue.push(2);
    queue.push(3);

    double d;
    queue.pop(d);
    ASSERT_DOUBLE_EQ(d, 2);
    queue.pop(d);
    ASSERT_DOUBLE_EQ(d, 3);
} 

TEST(TestQueue, SimplePushOverflow) {

    Queue<double, 2> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);

    double d;
    queue.pop(d);
    ASSERT_DOUBLE_EQ(d, 3);
} 



TEST(TestQueue, MultiThreading) {
    Queue<double, 32> q;

    constexpr int NUM_PRODUCERS = 1;
    constexpr int NUM_CONSUMERS = 1;
    constexpr int ITEMS_PER_PRODUCER = 1000;
    constexpr double POISON_PILL = std::numeric_limits<double>::quiet_NaN();


    std::atomic<int> total_produced{0};
    double consumed_sum = 0.0;
    std::mutex sum_mutex;

    // Producer function
    auto producer = [&](int items) {
        for (int i = 0; i < items; ++i) {
            q.push(1.0);  // Push actual data
            total_produced.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // Consumer function
    auto consumer = [&]() {
        double local_sum = 0.0;
        while (true) {
            double value;
            if(!q.pop(value)) continue;
            
            if (std::isnan(value)) {  // Check for poison pill
                break;
            }
            
            local_sum += value;
        }
        
        std::lock_guard<std::mutex> lock(sum_mutex);
        consumed_sum += local_sum;
    };

    // Start consumers
    std::vector<std::thread> consumers;
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(consumer);
    }

    // Start producers
    std::vector<std::thread> producers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer, ITEMS_PER_PRODUCER);
    }

    // Wait for producers to finish
    for (auto& t : producers) {
        t.join();
    }

    // Add poison pills to stop consumers
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        q.push(POISON_PILL);
    }

    // Wait for consumers to finish
    for (auto& t : consumers) {
        t.join();
    }

    // Verify results
    const double expected_total = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    ASSERT_DOUBLE_EQ(consumed_sum, expected_total);
    ASSERT_EQ(total_produced.load(), expected_total);
}


