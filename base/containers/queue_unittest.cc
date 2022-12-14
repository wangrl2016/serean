//
// Created by wangrl2016 on 2022/10/19.
//

#include <gtest/gtest.h>
#include "base/containers/queue.h"

// A sample program demonstrating using Google C++ testing framework.

// In this example, we use a more advanced feature of Google Test called
// test fixture.
//
// A test fixture is a place to hold objects and functions shared by
// all tests in a test case.  Using a test fixture avoids duplicating
// the test code necessary to initialize and cleanup those common
// objects for each test.  It is also useful for defining sub-routines
// that your tests need to invoke a lot.
//
// <TechnicalDetails>
//
// The tests share the test fixture in the sense of code sharing, not
// data sharing.  Each test is given its own fresh copy of the
// fixture.  You cannot expect the data modified by one test to be
// passed on to another test, which is a bad idea.
//
// The reason for this design is that tests should be independent and
// repeatable.  In particular, a test should not fail as the result of
// another test's failure.  If one test depends on info produced by
// another test, then the two tests should really be one big test.
//
// The macros for indicating the success/failure of a test
// (EXPECT_TRUE, FAIL, etc.) need to know what the current test is
// (when Google Test prints the test result, it tells you which test
// each failure belongs to).  Technically, these macros invoke a
// member function of the Test class.  Therefore, you cannot use them
// in a global function.  That's why you should put test sub-routines
// in a test fixture.
//
// </TechnicalDetails>

namespace base {
    // To use a test fixture, derive a class from testing::Test.
    class QueueTest : public testing::Test {
    protected:  // You should make the members protected s.t. they can be
        // accessed from subclasses.
        // virtual void SetUp() will be called before each test is run.  You
        // should define it if you need to initialize the variables.
        // Otherwise, this can be skipped.
        void SetUp() override {
            q1_.Enqueue(1);
            q2_.Enqueue(2);
            q2_.Enqueue(3);
        }

        // virtual void TearDown() will be called after each test is run.
        // You should define it if there is cleanup work to do.  Otherwise,
        // you don't have to provide it.
        //
        // virtual void TearDown() {
        // }

        // A helper function that some test uses.
        static int Double(int n) { return 2 * n; }

        // A helper function for testing Queue::Map().
        void MapTester(const Queue<int>* q) {
            // Creates a new queue, where each element is twice as big as the
            // corresponding one in q.
            const Queue<int>* const new_q = q->Map(Double);

            // Verifies that the new queue has the same size as q.
            ASSERT_EQ(q->Size(), new_q->Size());

            // Verifies the relationship between the elements of the two queues.
            for (const QueueNode<int>* n1 = q->Head(), * n2 = new_q->Head();
                 n1 != nullptr; n1 = n1->next(), n2 = n2->next()) {
                EXPECT_EQ(2 * n1->element(), n2->element());
            }

            delete new_q;
        }

        // Declares the variables your tests want to use.
        Queue<int> q0_;
        Queue<int> q1_;
        Queue<int> q2_;
    };

    // When you have a test fixture, you define a test using TEST_F
    // instead of TEST.

    // Tests the default c'tor.
    TEST_F(QueueTest, DefaultConstructor) {
        // You can access data in the test fixture here.
        EXPECT_EQ(0u, q0_.Size());
    }

    // Tests Dequeue().
    TEST_F(QueueTest, Dequeue) {
        int* n = q0_.Dequeue();
        EXPECT_TRUE(n == nullptr);

        n = q1_.Dequeue();
        ASSERT_TRUE(n != nullptr);
        EXPECT_EQ(1, *n);
        EXPECT_EQ(0u, q1_.Size());
        delete n;

        n = q2_.Dequeue();
        ASSERT_TRUE(n != nullptr);
        EXPECT_EQ(2, *n);
        EXPECT_EQ(1u, q2_.Size());
        delete n;
    }

    // Tests the Queue::Map() function.
    TEST_F(QueueTest, Map) {
        MapTester(&q0_);
        MapTester(&q1_);
        MapTester(&q2_);
    }
}
