#ifndef _threads_h_
#define _threads_h_

#include "atomic.h"
#include "queue.h"
#include "heap.h"
#include "debug.h"
#include "smp.h"

extern void threadsInit();

extern void stop();
extern void yield();

#define STACK_SIZE_IN_BYTES (8 * 1024)
#define STACK_SIZE_IN_UINT32 (STACK_SIZE_IN_BYTES / sizeof(uint32_t))

namespace dang0925 {

    void entry();

    struct TCB {

        // interrupt should be true
        TCB* next;
        bool should_delete = false;
        bool disable_flag = false;
        bool should_sleep = false;
        bool check_lock = false;
        ISL* internal_lock;
        Queue<TCB, NoLock> *blocking_queue;



        uint32_t saved_esp;

        virtual void do_the_work() {
            Debug::panic("Should never get here\n");
        }

        virtual ~TCB(){

        }
    };

    template <typename T>
    struct TCBWithWork: TCB {

        T work;

        uint32_t the_stack[STACK_SIZE_IN_UINT32] __attribute__ ((aligned(16)));
        TCBWithWork(T work): work(work) {
            the_stack[STACK_SIZE_IN_UINT32 - 1 ] = uint32_t(entry);
            saved_esp = uint32_t(&the_stack[STACK_SIZE_IN_UINT32 - 5]);
        }

        void do_the_work() override {
            work();
            stop();
        }
    };

    extern Queue<TCB, InterruptSafeLock> ready_queue;
    extern TCB* active[4];
    extern TCB* prev_active_tcb[4];

}
    extern void block_delete();

    extern void block(Queue<dang0925::TCB,NoLock>*,ISL*,bool);

template <typename T>

void thread(T work) {
    using namespace dang0925;
    block_delete();
    auto tcb = new TCBWithWork(work);
    ready_queue.add(tcb);
}

#endif