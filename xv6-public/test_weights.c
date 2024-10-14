#include "types.h"
#include "user.h"

#define NUM_PROCS 10

int
main(void)
{
  int pids[NUM_PROCS];
  int nice_values[NUM_PROCS] = {-25, -20, -15, -10, -5, 0, 5, 10, 15, 20};
  int weight_values[NUM_PROCS] = {88817, 88817, 29103, 9536, 3124, 1024, 335, 109, 36, 14};
  struct proc_info info[NUM_PROCS];
  int i, j;

  for(i = 0;i < NUM_PROCS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      setnice(getpid(), nice_values[i]);
      for(j = 0; j < 100000000; j++){
        asm volatile("nop");
      }
      exit();
    }
  }
  // Parent process
  sleep(5);  // Ensure children has been allocated
  for(i = 0; i < NUM_PROCS; i++){
    if(getprocinfo(pids[i], &info[i]) < 0){
      printf(1, "Error: getprocinfo failed\n");
      exit();
    }
    if(info[i].weight == weight_values[i]){
      printf(1, "Test Passed: Weight computed correctly for process %d\n", pids[i]);
    } else {
      printf(1, "Test Failed: Expected weight %d, got %d for process %d\n", weight_values[i], info[i].weight, pids[i]);
    }
  }

  for(i = 0; i < NUM_PROCS; i++){
    wait();
  }

  printf(1, "Test completed\n");

  exit();
}
