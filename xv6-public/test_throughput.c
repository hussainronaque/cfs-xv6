#include "types.h"
#include "user.h"

#define SMALL_WORKLOAD 500000000
#define LARGE_WORKLOAD 5000000000
#define NUM_SMALL_PROCS 30
#define NUM_LARGE_PROCS 20

int
main(void)
{
  int i, j;
  int total_procs = NUM_LARGE_PROCS+NUM_SMALL_PROCS;
  int pids[total_procs];
  struct proc_info info[total_procs];

  printf(1, "Starting Throughput Test\n");

  // Launch small workload processes
  for(i = 0; i < NUM_SMALL_PROCS; i++){
    pids[i] = fork();
    if(pids[i] == 0){
      // Child process
      for(j = 0; j < SMALL_WORKLOAD; j++){
        asm volatile("nop");
      }
      exit();
    }
  }

  // Launch large workload processes
  for(i = NUM_SMALL_PROCS; i < total_procs; i++){
    pids[i] = fork();
    if(pids[i] == 0){
      // Child process
      for(j = 0; j < LARGE_WORKLOAD; j++){
        asm volatile("nop");
      }
      exit();
    }
  }

  sleep(300);  // Allow processes to run

  int total_vruntime = 0;

  for (i = 0; i < total_procs; i++) {
    getprocinfo(pids[i], &info[i]);
    total_vruntime += info[i].vruntime;
  }

  int avg_vruntime = total_vruntime / (total_procs);
  printf(1, "Total VRuntime: %d, Average VRuntime: %d\n", total_vruntime, avg_vruntime);

  int passed = 1;
  for (i = 0; i < total_procs; i++) {
    int vruntime_i = (int)info[i].vruntime;
    int vruntime_f = (int)((info[i].vruntime-vruntime_i)*10000);
    int diff = (int)info[i].vruntime - avg_vruntime;
    if (diff < 0) diff = -diff;
    printf(1, "Process %d - VRuntime: %d.%d, difference from avg: %d\n", info[i].pid, vruntime_i, vruntime_f, diff);
    if (diff > avg_vruntime * 0.2){
      passed = 0;
      printf(1, "Test Failed: Process %d has VRuntime significantly different from average\n", info[i].pid);
    }
  }

  for (i = 0; i < total_procs; i++) {
    kill(pids[i]);
  }

  for (i = 0; i < total_procs; i++) {
    wait();
  }

  // Assertions
  if(passed){
    printf(1, "Test Passed: Throughput measured successfully\n");
  } else {
    printf(1, "Test Failed: Throughput measurement failed\n");
  }

  printf(1, "Throughput Test completed\n");
  exit();
}