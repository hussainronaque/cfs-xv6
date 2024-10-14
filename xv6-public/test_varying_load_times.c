#include "types.h"
#include "user.h"

#define INITIAL_PROCS 7
#define LATER_PROCS 4
#define WORKLOAD 500000000

int
main(void)
{
  int pids[INITIAL_PROCS + LATER_PROCS];
  int i, j;
  int total_procs = INITIAL_PROCS + LATER_PROCS;
  int exec_times[INITIAL_PROCS + LATER_PROCS];
  int pipefd[2];  // File descriptors for the pipe

  printf(1, "Starting Fairness Under Varying Loads Test\n");

  // Create a pipe for IPC
  if(pipe(pipefd) < 0){
    printf(1, "Pipe creation failed\n");
    exit();
  }

  // Fork initial processes
  for(i = 0; i < INITIAL_PROCS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      // Child process
      close(pipefd[0]);  // Close read end in child

      // Simulate workload
      int start_time = uptime();
      printf(1, "Initial Process %d started at %d ticks\n", getpid(), start_time);
      for(j = 0; j < WORKLOAD; j++){
        asm volatile("nop");
      }
      int end_time = uptime();
      int exec_time = end_time - start_time;

      printf(1, "Initial Process %d ran for %d ticks\n", getpid(), exec_time);

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

  sleep(100);  // Delay before forking later processes

  // Fork later processes
  for(i = INITIAL_PROCS; i < total_procs; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      // Child process
      close(pipefd[0]);  // Close read end in child

      // Simulate workload
      int start_time = uptime();
      printf(1, "Later Process %d started at %d ticks\n", getpid(), start_time);
      for(j = 0; j < WORKLOAD; j++){
        asm volatile("nop");
      }
      int end_time = uptime();
      int exec_time = end_time - start_time;

      printf(1, "Later Process %d ran for %d ticks\n", getpid(), exec_time);

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

  // Read execution times from the pipe
  struct {
    int pid;
    int exec_time;
  } child_data;

  for(i = 0; i < total_procs; i++){
    read(pipefd[0], &child_data, sizeof(child_data));
    // Find the index corresponding to child_data.pid
    for(int k = 0; k < total_procs; k++){
      if(pids[k] == child_data.pid){
        exec_times[k] = child_data.exec_time;
        break;
      }
    }
  }

  close(pipefd[0]);  // Close read end

  // Wait for all children to finish
  for(i = 0; i < total_procs; i++){
    wait();
  }

  // Assertions
  int total_exec_time = 0;
  for(i = 0; i < total_procs; i++){
    total_exec_time += exec_times[i];
  }

  int avg_exec_time = total_exec_time / total_procs;

  printf(1, "Total Execution Time: %d, Average Execution Time: %d\n", total_exec_time, avg_exec_time);

  int passed = 1;
  for(i = 0; i < total_procs; i++){
    int diff = exec_times[i] - avg_exec_time;
    if(diff < 0) diff = -diff;
    if(diff > avg_exec_time * 0.2){  // Allow 20% margin due to varying loads
      passed = 0;
      printf(1, "Process with PID %d: Expected ~%d ticks, got %d ticks\n", pids[i], avg_exec_time, exec_times[i]);
    }
  }

  if(passed){
    printf(1, "Test Passed: Fairness maintained under varying loads\n");
  } else {
    printf(1, "Test Failed: Fairness not maintained under varying loads\n");
  }

  printf(1, "Fairness Under Varying Loads Test completed\n");
  exit();
}