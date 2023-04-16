#ifndef _blocking_lock_h_
#define _blocking_lock_h_

#include "debug.h"
#include "threads.h"
#include "queue.h"
#include "atomic.h"


class BlockingLock {
    
    volatile bool taken;
    Queue<dang0925::TCB, NoLock> blocking_queue;
    ISL internal_lock;


public:
    // create a queue of all sleeping proccesses
    BlockingLock() : taken(false) {}

    void lock(void) {
        using namespace dang0925;
        while(taken){
            yield();
        }
        bool lock_state = internal_lock.lock();
        while(taken){
            block(&blocking_queue,&internal_lock,lock_state);
            internal_lock.lock();
        }
        taken = true;
        internal_lock.unlock(lock_state);
    }

    void unlock(void) {
        using namespace dang0925;
        bool lock_state = internal_lock.lock();
        auto unlocked = blocking_queue.remove();
        if(unlocked != nullptr){
            unlocked->should_sleep = false;
            ready_queue.add(unlocked);
        }
        taken = false;
        internal_lock.unlock(lock_state);
    }
    bool isMine() { return true; }
};



#endif

