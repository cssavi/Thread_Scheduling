#include "debug.h"
#include "threads.h"
#include "barrier.h"

/* Called by one CPU */
void kernelMain(void) {

    auto bar = new Barrier(5);
    Atomic<int> sum{0};

    for (int i=0; i<4; i++) {
        thread([bar,i,&sum] {
            sum.add_fetch(i+1);
            bar->sync();
        });
    }

    bar->sync();

    Debug::printf("*** sum = %d\n",sum.get());

}

