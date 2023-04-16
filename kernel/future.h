#ifndef _future_h_
#define _future_h_

#include "debug.h"
#include "threads.h"
#include "queue.h"
#include "atomic.h"

template <typename T>
class Future {
    bool lock_state;
    volatile bool ready;
    T value;
    Queue<dang0925::TCB, NoLock> check;
    ISL internal_lock;

public:

    Future() {
        ready = false;
        value = 0;
    }
    // Can't copy a future
    Future(const Future&) = delete;
    
    void set(T v) {
        using namespace dang0925;
        lock_state = internal_lock.lock();
        value = v;
        ready = true;
        auto unlocked = check.remove();
        while(unlocked != nullptr){
            unlocked->should_sleep = false;
            ready_queue.add(unlocked);
            unlocked = check.remove();
        }
        internal_lock.unlock(lock_state);
    }

    T get() {
        lock_state = internal_lock.lock();
        while(!ready){
            block(&check,&internal_lock,lock_state);
            internal_lock.lock();
        }
        internal_lock.unlock(lock_state);
        return value;
    }
};

#endif

