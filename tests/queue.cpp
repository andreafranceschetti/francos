
#include <francos/queue.hpp>
#include <gtest/gtest.h>


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



