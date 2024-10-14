#include "types.h"
#include "user.h"

#define NUM_INITIAL_PROCS 5
#define WORKLOAD 10000000
#define SLEEP_DURATION 200  // How long the child will sleep

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

  printf(1, "Starting vruntime Sleep/Wake Test with initial processes\n");

  //Create initial running processes
  for (i = 0; i < NUM_INITIAL_PROCS; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      printf(1, "Fork failed\n");
      exit();
    }

    if (pids[i] == 0) {  // Child process
      if (i == 0) {
        // This child will perform the sleep/wake test
        // Step 1: Do some work
        for (j = 0; j < WORKLOAD; j++) {
          asm volatile("nop");
        }

        // Step 2: Record the vruntime before sleeping
        getprocinfo(getpid(), &pinfo);
        double vruntime_before_sleep = pinfo.vruntime;
        printf(1, "Process %d vruntime before sleep: %d.%d\n", getpid(),
               (int)vruntime_before_sleep, (int)((vruntime_before_sleep - (int)vruntime_before_sleep) * 10000));

        // Step 3: Put the process to sleep
        sleep(SLEEP_DURATION);

        // Step 4: Record the vruntime after waking up
        getprocinfo(getpid(), &pinfo);

        // Step 5: Read the RB tree and find the minimum vruntime in the system
        int tree_node_count = gettreenodes(64, nodes);
        int min_pid = find_min_vruntime_in_tree(nodes, tree_node_count);
        getprocinfo(min_pid, &pinfo);
        double min_vruntime = pinfo.vruntime;

        double vruntime_after_wake = pinfo.vruntime;
        printf(1, "Process %d vruntime after wake: %d.%d\n", getpid(),
               (int)vruntime_after_wake, (int)((vruntime_after_wake - (int)vruntime_after_wake) * 10000));

        // Ensure that the vruntime after waking up is greater than before
        if (vruntime_after_wake > vruntime_before_sleep) {
          printf(1, "Test Passed: vruntime increased after wake-up\n");
        } else {
          printf(1, "Test Failed: vruntime did not increase correctly after wake-up\n");
        }

        // Compare the wake-up process's vruntime with the minimum vruntime
        printf(1, "Minimum vruntime in the tree after wake-up: %d.%d\n", (int)min_vruntime,
               (int)((min_vruntime - (int)min_vruntime) * 10000));

        if (vruntime_after_wake > 0.9 * min_vruntime &&
            vruntime_after_wake < 1.1 * min_vruntime) {
          printf(1, "Test Passed: Process's vruntime matches the minimum vruntime\n");
        } else {
          printf(1, "Test Failed: Process's vruntime does not match the minimum vruntime\n");
        }

        exit();  // Exit the child process after completing the test
      } else {
        // Other children will just run workload
        while (1) {
          for (j = 0; j < WORKLOAD; j++) {
            asm volatile("nop");
          }
        }
      }
    }
  }

  // Parent process waits for the test child to complete its test
  wait();  // Wait for the test child to finish

  // Cleanup: Kill the remaining running child processes
  for (i = 1; i < NUM_INITIAL_PROCS; i++) {
    kill(pids[i]);
    wait();
  }

  printf(1, "Vruntime Sleep/Wake Test completed\n");
  exit();
}

