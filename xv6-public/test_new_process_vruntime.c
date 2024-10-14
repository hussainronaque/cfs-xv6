#include "types.h"
#include "user.h"

#define NUM_INITIAL_PROCS 5
#define WORKLOAD 10000000

int find_min_vruntime_in_tree(struct rb_node_info *nodes, int count) {
  double min_vruntime = nodes[0].vruntime;
  int min_pid = nodes[0].pid;

  for (int i = 1; i < count; i++) {
    if (nodes[i].vruntime < min_vruntime) {
      min_vruntime = nodes[i].vruntime;
      min_pid = nodes[i].pid;
    }
  }
  return min_pid;
}

int main(void) {
  int pids[NUM_INITIAL_PROCS];
  int i, j;
  struct proc_info pinfo;
  struct rb_node_info nodes[64];  // Arbitrary max size for RB tree nodes
  int pipe_fd[2];  // Pipe for communication between child and parent

  printf(1, "Starting vruntime Test with initial processes\n");

  // Step 1: Create initial running processes
  for (i = 0; i < NUM_INITIAL_PROCS; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      printf(1, "Fork failed\n");
      exit();
    }

    if (pids[i] == 0) {  // Child process
      // Simulate workload in each child
      while (1) {
        for (j = 0; j < WORKLOAD; j++) {
          asm volatile("nop");
        }
      }
    }
  }

  // Step 2: Let children run for some time
  sleep(500);  // Give them time to accumulate some vruntime

  // Step 3: Read the RB tree to find the process with the minimum vruntime
  int tree_node_count = gettreenodes(64, nodes);  // Get info about RB tree nodes
  // Step 4: Immediately create a pipe and fork a new process after reading the RB tree
  if (pipe(pipe_fd) < 0) {
    printf(1, "Pipe failed\n");
    exit();
  }

  int pid_new = fork();
  if (pid_new < 0) {
    printf(1, "Fork failed\n");
    exit();
  }

  if (pid_new == 0) {  // New process (child)
    // Step 5: Immediately get proc_info for this new process
    getprocinfo(getpid(), &pinfo);
    

    // Step 6: Read the vruntime of the new process from the pipe
    double new_vruntime;
    new_vruntime = pinfo.vruntime;
    
    // Step 7: Get the proc_info of the process with the minimum vruntime
    int min_pid = find_min_vruntime_in_tree(nodes, tree_node_count);
    getprocinfo(min_pid, &pinfo);
    double min_vruntime = pinfo.vruntime;

    // Step 8: Compare vruntime of the new process with the minimum vruntime
    int new_vruntime_i = (int)new_vruntime;
    int new_vruntime_f = (int)((new_vruntime-new_vruntime_i)*10000);
    int min_vruntime_i = (int)min_vruntime;
    int min_vruntime_f = (int)((min_vruntime-min_vruntime_i)*10000);
    printf(1, "Comparing vruntimes: New Process VRuntime = %d.%d, Min VRuntime in Tree = %d.%d\n",
           new_vruntime_i, new_vruntime_f, min_vruntime_i, min_vruntime_f);

    if (new_vruntime != 0.0 &&
        new_vruntime > 0.9 * min_vruntime &&
        new_vruntime < 1.1 * min_vruntime) {
      printf(1, "Test Passed: New process vruntime matches the minimum vruntime in runnable processes\n");
    } else {
      printf(1, "Test Failed: New process vruntime does not match the minimum vruntime in runnable processes\n");
    }

    // Keep the process running for a while
    for (j = 0; j < WORKLOAD; j++) {
      asm volatile("nop");
    }
    exit();
  } else {
    // Parent process
    
    // Cleanup: Kill the remaining running child processes
    for (i = 0; i < NUM_INITIAL_PROCS; i++) {
      kill(pids[i]);
      wait();
    }

    // Don't wait for the new process to terminate, as we're only concerned with vruntime
    printf(1, "Vruntime Test completed\n");
    exit();
  }
}

