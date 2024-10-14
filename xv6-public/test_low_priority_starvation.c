// In user/test_low_priority_starvation.c
#include "types.h"
#include "user.h"

#define WORKLOAD 100000000

int
main(void)
{
  int pid_high, pid_low;
  int pipe_fds[2];  // Pipe for communication between the low-priority process and parent
  int start_time_low, end_time_low;

  printf(1, "Starting Low-Priority Starvation Test\n");

  // Create a pipe for communication
  if (pipe(pipe_fds) < 0) {
    printf(1, "Pipe creation failed\n");
    exit();
  }

  // High priority process
  pid_high = fork();
  if (pid_high < 0) {
    printf(1, "Fork failed\n");
    exit();
  }

  if (pid_high == 0) {
    // High-priority child process
    setnice(getpid(), 0);
    // Infinite loop to keep it running
    while(1) {
      asm volatile("nop");
    }
    exit();
  }

  // Low priority process
  pid_low = fork();
  if (pid_low < 0) {
    printf(1, "Fork failed\n");
    exit();
  }

  if (pid_low == 0) {
    // Low-priority child process

    // Close the reading end of the pipe in the child
    close(pipe_fds[0]);

    setnice(getpid(), -10);

    // Simulate workload
    start_time_low = uptime();
    for (int j = 0; j < WORKLOAD; j++) {
      asm volatile("nop");
    }
    end_time_low = uptime();

    // Send the execution time to the parent via the pipe
    int exec_time_low = end_time_low - start_time_low;
    write(pipe_fds[1], &exec_time_low, sizeof(exec_time_low));

    // Close the writing end of the pipe
    close(pipe_fds[1]);

    exit();
  }

  // Parent process

  // Close the writing end of the pipe in the parent
  close(pipe_fds[1]);

  // Sleep to allow the processes to run
  sleep(500);  // Let the children run for a while

  // Kill the high-priority process
  kill(pid_high);
  wait();  // Wait for the high-priority process to exit
  wait();  // Wait for the low-priority process to exit

  // Read the execution time from the low-priority process
  int exec_time_low = 0;
  read(pipe_fds[0], &exec_time_low, sizeof(exec_time_low));

  // Close the reading end of the pipe in the parent
  close(pipe_fds[0]);

  // Assertion
  if (exec_time_low > 0) {
    printf(1, "Test Passed: Low-priority process received CPU time (ran for %d ticks)\n", exec_time_low);
  } else {
    printf(1, "Test Failed: Low-priority process did not receive CPU time (starved)\n");
  }

  printf(1, "Low-Priority Starvation Test completed\n");
  exit();
}

