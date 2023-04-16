#include "debug.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "machine.h"
#include "threads.h"

namespace dang0925 {
    // change to ISL
    Queue<TCB, InterruptSafeLock> ready_queue{};
    Queue<TCB, InterruptSafeLock> delete_queue{};
    TCB* active[4];
    TCB* prev_active_tcb[4];

    void entry() {
        if(prev_active_tcb[SMP::me()]->should_sleep){
            prev_active_tcb[SMP::me()]->blocking_queue->add(prev_active_tcb[SMP::me()]);            
            prev_active_tcb[SMP::me()]->internal_lock->unlock(prev_active_tcb[SMP::me()]->check_lock);
            prev_active_tcb[SMP::me()]->should_sleep = false;
        } else if(prev_active_tcb[SMP::me()]->should_delete){
            delete_queue.add(prev_active_tcb[SMP::me()]);
            //block_delete();
        } else {
            ready_queue.add(prev_active_tcb[SMP::me()]);
            prev_active_tcb[SMP::me()] = nullptr;
        }
        prev_active_tcb[SMP::me()] = nullptr;
        auto active_thread = active[SMP::me()];
        Interrupts::restore(false);
        active_thread->do_the_work();
        stop();
        }
    }

void threadsInit() {
    using namespace dang0925;
    for(int i = 0;i<4;i++){
        TCB* dummy_thread = new TCB();
        dummy_thread->should_delete = true;
        active[i] = new TCB();
    }
}


extern "C" void context_switch(uint32_t,uint32_t&);


void yield() {
    using namespace dang0925;
    bool checker = Interrupts::disable();
    auto target = ready_queue.remove();
    if (target == nullptr) {
        Interrupts::restore(checker);
        return;
    }
    active[SMP::me()]->disable_flag = checker;
    prev_active_tcb[SMP::me()] = active[SMP::me()];
    active[SMP::me()] = target;    
    context_switch(target->saved_esp,prev_active_tcb[SMP::me()]->saved_esp);
    if(prev_active_tcb[SMP::me()]->should_sleep){
        prev_active_tcb[SMP::me()]->blocking_queue->add(prev_active_tcb[SMP::me()]);            
        prev_active_tcb[SMP::me()]->internal_lock->unlock(prev_active_tcb[SMP::me()]->check_lock);
        prev_active_tcb[SMP::me()]->should_sleep = false;
    } else if(prev_active_tcb[SMP::me()]->should_delete){
        delete_queue.add(prev_active_tcb[SMP::me()]);
    } else {
        ready_queue.add(prev_active_tcb[SMP::me()]);
    }
    prev_active_tcb[SMP::me()] = nullptr;
    Interrupts::restore(false);
}    

void block_delete(){
    using namespace dang0925;
    //delete delete_queue.remove();
    auto terminate = delete_queue.remove();
    while(terminate!=nullptr){
        delete terminate;
        terminate = delete_queue.remove();
    }
}

using namespace dang0925;
void block(Queue<TCB,NoLock>* blocking_queue,ISL* internal_lock, bool check){

    using namespace dang0925;
    bool checker = Interrupts::disable();
    auto target = ready_queue.remove();
    if (target == nullptr) {
        internal_lock->unlock(check);
        Interrupts::restore(checker);
        return;
    } else {
        prev_active_tcb[SMP::me()]->disable_flag = checker;
        prev_active_tcb[SMP::me()] = active[SMP::me()];
        active[SMP::me()] = target;    
        prev_active_tcb[SMP::me()]->should_sleep = true;
        prev_active_tcb[SMP::me()]->check_lock = check;
        prev_active_tcb[SMP::me()]->internal_lock = internal_lock;
        prev_active_tcb[SMP::me()]->blocking_queue = blocking_queue;
        context_switch(target->saved_esp,prev_active_tcb[SMP::me()]->saved_esp);
        if(prev_active_tcb[SMP::me()]->should_sleep){
            prev_active_tcb[SMP::me()]->blocking_queue->add(prev_active_tcb[SMP::me()]);
            prev_active_tcb[SMP::me()]->internal_lock->unlock(prev_active_tcb[SMP::me()]->check_lock);
            prev_active_tcb[SMP::me()]->should_sleep = false;
        } else if(prev_active_tcb[SMP::me()]->should_delete){
            delete_queue.add(prev_active_tcb[SMP::me()]);
        } else {
            ready_queue.add(prev_active_tcb[SMP::me()]);
        }
        prev_active_tcb[SMP::me()] = nullptr;
    }
    Interrupts::restore(false);
}

void stop() {
    using namespace dang0925;
    bool check_disable = Interrupts::disable();
    active[SMP::me()]->should_delete=true;
    active[SMP::me()]->disable_flag=check_disable;
    //prev_active_tcb[SMP::me()]->should_delete=true;
    while(true){
        yield();
    }
}
