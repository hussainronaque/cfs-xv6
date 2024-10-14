// In user/test_different_priorities.c
#include "types.h"
#include "user.h"
#include "fcntl.h"

#define NUM_PROCS 3
#define WORKLOAD 100000000

int
main(void)
{
  int pids[NUM_PROCS];
  int nice_values[NUM_PROCS] = {0, 10, 20};
  int i, j;
  int start_time, end_time;
  int exec_times[NUM_PROCS];
  int pipe_fds[2];  // File descriptors for the pipe

  // Create a pipe for communication
  if (pipe(pipe_fds) < 0) {
    printf(1, "Pipe creation failed\n");
    exit();
  }

  printf(1, "Starting Different Priorities Test\n");

  for(i = 0; i < NUM_PROCS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      // Child process

      // Close reading end of the pipe in the child
      close(pipe_fds[0]);

      setnice(getpid(), nice_values[i]);

      // Simulate workload
      start_time = uptime();
      for(j = 0; j < WORKLOAD; j++){
        asm volatile("nop");
      }
      end_time = uptime();

      // Calculate execution time
      int exec_time = end_time - start_time;
      int pid = getpid();  // Get child process PID

      printf(1, "Process %d with nice value %d ran for %d ticks\n",
             pid, nice_values[i], exec_time);

      // Write PID and execution time to the pipe
      write(pipe_fds[1], &pid, sizeof(pid));
      write(pipe_fds[1], &exec_time, sizeof(exec_time));

      // Close writing end of the pipe in the child
      close(pipe_fds[1]);

      exit();
    }
  }

  // Close writing end of the pipe in the parent
  close(pipe_fds[1]);

  // Parent waits for all children to finish and collects their execution times
  for(i = 0; i < NUM_PROCS; i++){
    wait();  // Wait for child process to finish
  }

  // Read PIDs and execution times from the pipe
  for(i = 0; i < NUM_PROCS; i++){
    int pid;
    int exec_time;

    read(pipe_fds[0], &pid, sizeof(pid));         // Read PID
    read(pipe_fds[0], &exec_time, sizeof(exec_time));  // Read execution time

    // Match PID to the index and store the execution time in the correct spot
    for (int k = 0; k < NUM_PROCS; k++) {
      if (pids[k] == pid) {
        exec_times[k] = exec_time;
        break;
      }
    }
  }

  // Close reading end of the pipe in the parent
  close(pipe_fds[0]);

  // Assertions
  int higher_priority_time = exec_times[0];
  int medium_priority_time = exec_times[1];
  int lower_priority_time = exec_times[2];

  if(higher_priority_time < medium_priority_time && medium_priority_time < lower_priority_time){
    printf(1, "Test Passed: Higher priority processes received more CPU time proportionally\n");
  } else {
    printf(1, "Test Failed: CPU time not allocated proportionally to priority\n");
  }

  printf(1, "Different Priorities Test completed\n");
  exit();
}

