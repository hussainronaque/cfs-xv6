// In user/test_response_time.c
#include "types.h"
#include "user.h"
#include "fcntl.h"

#define NUM_PROCS 50
#define WORKLOAD 100000000

int
main(void)
{
  int pids[NUM_PROCS];
  int create_time[NUM_PROCS];
  int response_time[NUM_PROCS];
  int pipe_fds[2];  // Pipe for passing start times and response times
  int nice_values[] = {-20, -10, 0, 10, 19};  // Wide range of nice values
  int i;

  // Create a pipe for communication
  if (pipe(pipe_fds) < 0) {
    printf(1, "Pipe creation failed\n");
    exit();
  }

  printf(1, "Starting Response Time Test\n");

  // Fork all child processes with different nice values
  for(i = 0; i < NUM_PROCS; i++){
    create_time[i] = uptime();  // Record process creation time
    pids[i] = fork();
    if(pids[i] < 0){
      printf(1, "Fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      // Child process
      // Record start time and calculate response time
      int start_time = uptime();
      int resp_time = start_time - create_time[i];  // Calculate response time
      int pid = getpid();

      // Close reading end of the pipe in the child
      close(pipe_fds[0]);

      // Set the nice value of the child
      setnice(getpid(), nice_values[i % 5]);
      printf(1, "Process %d started with nice value %d\n", getpid(), nice_values[i % 5]);

      // Simulate workload for high priority tasks
      for (int j = 0; j < WORKLOAD; j++) {
        asm volatile("nop");
      }

      // Write the response time to the pipe
      write(pipe_fds[1], &pid, sizeof(pid));  // Send PID
      write(pipe_fds[1], &resp_time, sizeof(resp_time));  // Send response time

      // Close writing end of the pipe in the child
      close(pipe_fds[1]);

      exit();  // Exit child process
    }
  }

  // Close writing end of the pipe in the parent
  close(pipe_fds[1]);

  // Parent waits for all children to finish
  for(i = 0; i < NUM_PROCS; i++){
    wait();
  }

  // Parent reads the response times from the pipe
  for(i = 0; i < NUM_PROCS; i++){
    int pid;
    int resp_time;

    // Read PID and response time from the pipe
    read(pipe_fds[0], &pid, sizeof(pid));
    read(pipe_fds[0], &resp_time, sizeof(resp_time));

    // Find the corresponding process and store its response time
    for(int j = 0; j < NUM_PROCS; j++){
      if(pids[j] == pid){
        response_time[j] = resp_time;
        printf(1, "Process %d with nice value %d had a response time of %d ticks\n", pid, nice_values[j % 5], response_time[j]);
        break;
      }
    }
  }

  // Close reading end of the pipe in the parent
  close(pipe_fds[0]);

  // Assertions: Find the maximum response time
  int max_response_time = 0;
  for(i = 0; i < NUM_PROCS; i++){
    if(response_time[i] > max_response_time){
      max_response_time = response_time[i];
    }
  }

  if(max_response_time <= NUM_PROCS*2 + NUM_PROCS/5){  // response time should be within the latency
    printf(1, "Test Passed: Processes scheduled promptly after creation\n");
  } else {
    printf(1, "Test Failed: Some processes experienced high response time\n");
  }

  printf(1, "Response Time Test completed\n");
  exit();
}

