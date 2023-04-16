#ifndef _bb_h_
#define _bb_h_

#include "debug.h"
#include "stdint.h"

    // This implementation of BoundedBuffer is from the textbook
template <typename T>
class BoundedBuffer {
    T* val_array;
    volatile uint32_t in_index;
    volatile uint32_t out_index;
    volatile uint32_t max;
    ISL internal_lock;

public:

    Queue<dang0925::TCB, NoLock> producer_queue;
    Queue<dang0925::TCB, NoLock> consumer_queue;

    BoundedBuffer(uint32_t N)  {
        val_array = new T[N];
        max = N;
        in_index=0;
        out_index=0;
    }

    ~BoundedBuffer() {
    }

    BoundedBuffer(const BoundedBuffer&) = delete;

    void put(T t) {    
        using namespace dang0925;
        bool lock_state = internal_lock.lock();
        while(out_index - in_index == max){
            block(&producer_queue,&internal_lock,lock_state);
            internal_lock.lock();
        } 
        val_array[out_index%max]=t;
        out_index++;
        auto prod = consumer_queue.remove();
        if(prod != nullptr){
            prod->should_sleep = false;
            ready_queue.add(prod);
        }
        internal_lock.unlock(lock_state);
    }
  
    T get() {
        using namespace dang0925;
        bool lock_state = internal_lock.lock();
        while(out_index == in_index){
            block(&consumer_queue,&internal_lock,lock_state);
            internal_lock.lock();
        } 
        T temp;
        temp = val_array[in_index%max];
        in_index++;
        auto cons = producer_queue.remove();
        if(cons != nullptr){
            cons->should_sleep = false;
            ready_queue.add(cons);
        }
        internal_lock.unlock(lock_state);
        return temp;
    }      
};

#endif
