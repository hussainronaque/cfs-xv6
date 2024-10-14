#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PROCS 7

int
main(void)
{
  int pids[NUM_PROCS];
  int nice_values[NUM_PROCS] = {-20, -15, -10, -5, 0, 5, 10};
  struct proc_info info[NUM_PROCS];
  int count, total_weight, period;
  int i, j;
  int start_time, end_time, init_time, response_time;

  printf(1, "Starting CFS scheduler test\n");
  init_time = uptime();

  for(i = 0; i < NUM_PROCS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      response_time = uptime() - init_time;
      printf(1, "Started process %d after %d ticks\n", getpid(), response_time);
      // Child process
      setnice(getpid(), nice_values[i]);

      // Simulate workload
      start_time = uptime();

      for(j = 0; j < 2000000000; j++){
        asm volatile("nop");
      }
      end_time = uptime();

      printf(1, "Process %d with nice value %d ran for %d ticks\n",
             getpid(), nice_values[i], end_time - start_time);
      getprocinfo(getpid(), &info[i]);
      int vruntime_integer_part = (int)info[i].vruntime;
      int vruntime_fractional_part = (int)((info[i].vruntime-vruntime_integer_part)*10000);
      printf(1, "Process %d Info - Nice Value: %d, Weight: %d, VRuntime: %d.%d, CurrRuntime: %d\n\n",
             info[i].pid, info[i].nice_value, info[i].weight, vruntime_integer_part, vruntime_fractional_part, info[i].curr_runtime);
      exit();
    }
  }

  // Parent waits for all children to finish
  
  sleep(10);
  gettreeinfo(&count, &total_weight, &period);
  printf(1, "Tree Info - Count: %d, Total Weight: %d, Period: %d\n\n", count, total_weight, period);

  for(i = 0; i < NUM_PROCS; i++){
    wait();
  }

  printf(1, "CFS scheduler test completed\n");
  exit();
}
