#ifndef _barrier_h_
#define _barrier_h_

#include "debug.h"
#include "threads.h"
#include "queue.h"
#include "atomic.h"


    // This implementation of Barrier is from the textbook
class Barrier {

    volatile uint32_t thread_counter;
    volatile uint32_t threads_entered;
    ISL internal_lock;
    Queue<dang0925::TCB, NoLock> check;    

public:

    Barrier(uint32_t count) {
        thread_counter = count;
        threads_entered= 0;
    }

    Barrier(const Barrier&) = delete;
    
    void sync() {
        using namespace dang0925;
        bool lock_state = internal_lock.lock();
        threads_entered++;
        if(threads_entered<thread_counter){
            while(threads_entered<thread_counter){
                block(&check,&internal_lock,lock_state);
                lock_state = internal_lock.lock();
            }
        } else {
            auto target = check.remove();
            while(target != nullptr){
                target->should_sleep = false;
                ready_queue.add(target);
                target = check.remove();
            }
        }
        internal_lock.unlock(lock_state);
    }
};

    // This implementation of ReusableBarrier is from the textbook

class ReusableBarrier {

    volatile uint32_t thread_counter;
    volatile uint32_t threads_entered;
    volatile uint32_t threads_leaving;
    ISL internal_lock;
    Queue<dang0925::TCB, NoLock> check;


public:

    ReusableBarrier(uint32_t count) {
        thread_counter = count;
        threads_entered = 0;
        threads_leaving = 0;
    }

    ReusableBarrier(const ReusableBarrier&) = delete;

        void sync() {
        using namespace dang0925;
        bool ILock = internal_lock.lock();
        threads_entered++;
        if(threads_entered<thread_counter){
            while(threads_entered<thread_counter){
                block(&check,&internal_lock,ILock);
                internal_lock.lock();
            }
        } else {
            threads_leaving = 0;
            auto target = check.remove();
            while(target != nullptr){
                target->should_sleep = false;
                ready_queue.add(target);
                target = check.remove();
            }
        }
        threads_leaving++;
        if(threads_leaving<thread_counter){
            while(threads_leaving<thread_counter){
                block(&check,&internal_lock,ILock);
                internal_lock.lock();
            }
        } else {
            threads_entered = 0;
            auto target = check.remove();
            while(target != nullptr){
                target->should_sleep = false;
                ready_queue.add(target);
                target = check.remove();
            }
        }
        internal_lock.unlock(ILock);
    }

};
        
        
        

#endif

