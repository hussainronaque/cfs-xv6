// In user/test_cpu_time_allocation.c
#include "types.h"
#include "user.h"
#include "fcntl.h"

#define NUM_PROCS 20
#define WORKLOAD 100000000

int
main(void)
{
  int pids[NUM_PROCS];
  int nice_values[NUM_PROCS];
  int pipe_fds[2];  // Pipe for communication between parent and children
  int i, j;
  int total_exec_time = 0;

  // Assign nice values from -20 to 10
  for(i = 0; i < NUM_PROCS; i++) {
    nice_values[i] = -20 + (i * 40 / NUM_PROCS);  // Spread values between -20 to 19
  }

  // Create a pipe for communication
  if (pipe(pipe_fds) < 0) {
    printf(1, "Pipe creation failed\n");
    exit();
  }

  printf(1, "Starting CPU Time Allocation Test with %d processes\n", NUM_PROCS);

  // Fork child processes
  for(i = 0; i < NUM_PROCS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      // Child process

      // Close the reading end of the pipe in the child
      close(pipe_fds[0]);

      // Set the nice value of the child
      setnice(getpid(), nice_values[i]);

      // Simulate workload
      int start_time = uptime();
      for(j = 0; j < WORKLOAD; j++){
        asm volatile("nop");
      }
      int end_time = uptime();
      int exec_time = end_time - start_time;

      // Write the PID and execution time to the pipe
      write(pipe_fds[1], &pids[i], sizeof(pids[i]));     // Send PID
      write(pipe_fds[1], &exec_time, sizeof(exec_time));  // Send execution time

      // Close the writing end of the pipe in the child
      close(pipe_fds[1]);

      printf(1, "Process %d with nice value %d ran for %d ticks\n", getpid(), nice_values[i], exec_time);
      exit();  // Exit child process
    }
  }

  // Close the writing end of the pipe in the parent
  close(pipe_fds[1]);

  // Parent process waits for all children to finish
  for(i = 0; i < NUM_PROCS; i++){
    wait();
  }

  // Initialize an array to store execution times received from children
  int exec_times[NUM_PROCS] = {0};

  // Parent reads the execution times from the pipe
  for(i = 0; i < NUM_PROCS; i++){
    int pid;
    int exec_time;

    // Read PID and execution time from the pipe
    read(pipe_fds[0], &pid, sizeof(pid));
    read(pipe_fds[0], &exec_time, sizeof(exec_time));

    // Find the corresponding process and store its execution time
    for(int j = 0; j < NUM_PROCS; j++){
      if(pids[j] == pid){
        exec_times[j] = exec_time;
        total_exec_time += exec_times[j];
        printf(1, "Process %d with nice value %d had an execution time of %d ticks\n", pid, nice_values[j], exec_times[j]);
        break;
      }
    }
  }

  // Close the reading end of the pipe in the parent
  close(pipe_fds[0]);

  // Expected ratios based on weights
  int expected_weight[NUM_PROCS];
  int total_weight = 0;
  for(i = 0; i < NUM_PROCS; i++){
    // Assuming weights are inversely proportional to nice values
    expected_weight[i] = 1024 / (1.25 * nice_values[i] + 1);  // +1 to avoid division by zero
    total_weight += expected_weight[i];
  }

  // Calculate expected execution times
  int expected_exec_time[NUM_PROCS];
  for(i = 0; i < NUM_PROCS; i++){
    expected_exec_time[i] = (total_exec_time * expected_weight[i]) / total_weight;
  }

  // Assertions
  int passed = 1;
  for(i = 0; i < NUM_PROCS; i++){
    int diff = exec_times[i] - expected_exec_time[i];
    if(diff < 0) diff = -diff;
    if(diff > total_exec_time * 0.1){  // Allow 10% margin
      passed = 0;
      printf(1, "Process %d: Expected %d ticks, got %d ticks\n", pids[i], expected_exec_time[i], exec_times[i]);
    }
  }

  if(passed){
    printf(1, "Test Passed: CPU time allocated proportionally to weights\n");
  } else {
    printf(1, "Test Failed: CPU time allocation not proportional to weights\n");
  }

  printf(1, "CPU Time Allocation Test completed\n");
  exit();
}

