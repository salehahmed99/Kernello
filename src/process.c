#include"clk.h"
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

void run_process(int runtime) {
    sync_clk();
    
    while (runtime--) {
        // Process is running
        sleep(1); // Small sleep to reduce CPU usage
    }
    printf("Process %d finished at time %d\n ", getpid() , get_clk());
    destroy_clk(0);
}