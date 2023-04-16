#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "config.h"
#include "smp.h"
#include "blocking_lock.h"

//This test checks your Blockinglock implementation can handle deadlocks

void kernelMain(){
    BlockingLock *b = new BlockingLock();
    Atomic<int> *counter = new Atomic<int>(0);
    for(int i = 0; i < 2; i++){
        thread([b, counter]{
        b->lock();
        counter->add_fetch(5);
        thread([b, counter]{
            b->lock();
            counter->add_fetch(5);
            b->unlock();
            });
        b->unlock();
        });

        thread([b, counter]{
        b->lock();
        counter->add_fetch(-1);
        thread([b, counter]{
            b->lock();
            counter->add_fetch(-1);
            b->unlock();
            });
        b->unlock();
        });
    }
    while(counter->get() != 16) yield();
    Debug::printf("*** counter = %d\n", counter->get());
}