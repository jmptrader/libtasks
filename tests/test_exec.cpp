/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/dispatcher.h>
#include <tasks/exec.h>
#include <tasks/executor.h>  // for set_timeout
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <set>

#include "test_exec.h"

std::mutex g_mutex;
std::condition_variable g_cond;

namespace tasks {

void test_exec::run() {
    std::atomic<int> state(0);

    // create a task
    auto t1 = new exec_task([&state] {
        state = 1;
        g_cond.notify_one();
    });
    dispatcher::instance()->add_task(t1);

    CPPUNIT_ASSERT_MESSAGE(std::string("state=") + std::to_string(state), check_state(state, 1));

    exec([&state] { state++; }, [&state] {
                                    state++;
                                    g_cond.notify_one();
                                });

    CPPUNIT_ASSERT_MESSAGE(std::string("state=") + std::to_string(state), check_state(state, 3));

    state = 0;
    for (int i = 0; i < 1000; i++) {
        exec([&state] {
            state++;
            g_cond.notify_one();
        });
    }

    CPPUNIT_ASSERT_MESSAGE(std::string("state=") + std::to_string(state), check_state(state, 1000));

    /* Deactivated as they are a bit unreliable because of all the sleeping.
    // dispatcher tests
    CPPUNIT_ASSERT_MESSAGE(std::string("size=") + std::to_string(dispatcher::instance()->m_executors.size()),
    dispatcher::instance()->m_executors.size() == 1);
    // save ref to the one executor
    std::shared_ptr<executor> executor1 = dispatcher::instance()->free_executor();
    executor1->m_busy = false;

    // use it
    exec([] { std::this_thread::sleep_for(std::chrono::seconds(2)); });

    // create a new executor
    std::shared_ptr<executor> executor2 = dispatcher::instance()->free_executor();
    executor2->m_busy = false;

    // they should be different
    CPPUNIT_ASSERT(executor1.get() != executor2.get());

    // use the second executor
    exec([] { std::this_thread::sleep_for(std::chrono::seconds(2)); });

    // check that we have two
    CPPUNIT_ASSERT_MESSAGE(std::string("size=") + std::to_string(dispatcher::instance()->m_executors.size()),
    dispatcher::instance()->m_executors.size() == 2);

    // now let they all die
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // free_executor will clean up and create a new executor
    std::shared_ptr<executor> executor3 = dispatcher::instance()->free_executor();
    executor3->m_busy = false;

    CPPUNIT_ASSERT(executor1.get() != executor3.get());
    CPPUNIT_ASSERT(executor2.get() != executor3.get());

    // we should be back to one now
    CPPUNIT_ASSERT_MESSAGE(std::string("size=") + std::to_string(dispatcher::instance()->m_executors.size()),
    dispatcher::instance()->m_executors.size() == 1);
    */
}

bool test_exec::check_state(std::atomic<int>& state, int expected) {
    std::unique_lock<std::mutex> lock(g_mutex);
    return g_cond.wait_for(lock, std::chrono::seconds(10), [&state, expected] { return state == expected; });
}
}
