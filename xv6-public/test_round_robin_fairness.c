#include "types.h"
#include "user.h"

#define NUM_PROCS 40
#define WORKLOAD 50000000

int
main(void)
{
  int pids[NUM_PROCS];
  int i, j;
  int exec_times[NUM_PROCS];
  int total_exec_time = 0;
  int pipefd[2];  // File descriptors for the pipe

  printf(1, "Starting Round-Robin Fairness Test\n");

  // Create a pipe for IPC
  if(pipe(pipefd) < 0){
    printf(1, "Pipe creation failed\n");
    exit();
  }

  for(i = 0; i < NUM_PROCS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      // Child process with default nice value
      close(pipefd[0]);  // Close read end in child

      // Simulate workload
      int start_time = uptime();
      for(j = 0; j < WORKLOAD; j++){
        asm volatile("nop");
      }
      int end_time = uptime();

      int exec_time = end_time - start_time;

      // Write execution time and PID to the pipe
      struct {
        int pid;
        int exec_time;
      } child_data;

      child_data.pid = getpid();
      child_data.exec_time = exec_time;

      write(pipefd[1], &child_data, sizeof(child_data));

      close(pipefd[1]);  // Close write end
      exit();
    }
  }

  // Parent process
  close(pipefd[1]);  // Close write end in parent

  // Parent waits for all children to finish
  for(i = 0; i < NUM_PROCS; i++){
    wait();
  }

  // Read execution times from the pipe
  struct {
    int pid;
    int exec_time;
  } child_data;

  for(i = 0; i < NUM_PROCS; i++){
    read(pipefd[0], &child_data, sizeof(child_data));
    // Find the index corresponding to child_data.pid
    for(int k = 0; k < NUM_PROCS; k++){
      if(pids[k] == child_data.pid){
        exec_times[k] = child_data.exec_time;
        break;
      }
    }
  }

  close(pipefd[0]);  // Close read end


  // Calculate total execution time
  for(i = 0; i < NUM_PROCS; i++){
    total_exec_time += exec_times[i];
  }

  // Calculate average execution time
  int avg_exec_time = total_exec_time / NUM_PROCS;

  printf(1, "Total Execution Time: %d, Average Execution Time: %d\n", total_exec_time, avg_exec_time);

  // Assertions
  int passed = 1;
  for(i = 0; i < NUM_PROCS; i++){
    printf(1, "Process %d: %d ticks\n", pids[i], avg_exec_time, exec_times[i]);
    int diff = exec_times[i] - avg_exec_time;
    if(diff < 0) diff = -diff;
    if(diff > total_exec_time * 0.1){  // Allow 10% margin
      passed = 0;
      printf(1, "Process %d: Expected ~%d ticks, got %d ticks\n", pids[i], avg_exec_time, exec_times[i]);
    }
  }

  if(passed){
    printf(1, "Test Passed: Equal-weight processes received approximately equal CPU time\n");
  } else {
    printf(1, "Test Failed: CPU time allocation unequal among equal-weight processes\n");
  }

  printf(1, "Round-Robin Fairness Test completed\n");
  exit();
}