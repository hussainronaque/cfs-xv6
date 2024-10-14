#include "types.h"
#include "user.h"

#define MAX_NODES 64 // NPROC

int
main(void)
{
  struct rb_node_info nodes[MAX_NODES];
  int pids[30];
  int num_nodes, i;

  // Create several processes
  for(i = 0; i < 30; i++){
    if(fork() == 0){
      pids[i] = getpid();
      setnice(pids[i], i-10);
      for(int j = 0; j < 250000000; j++){
        asm volatile("nop");
      }
      exit();
    }
  }

  sleep(1);  // Allow processes to be inserted

  num_nodes = gettreenodes(MAX_NODES, nodes);
  int balanced = treebalanced();

  printf(1, "Number of Nodes in RB Tree: %d, tree balanced: %d\n", num_nodes, balanced);

  // Verify BST property: In-order traversal should yield sorted virtual runtimes
  int sorted = 1;
  for(i = 1; i < num_nodes; i++){
    int vruntime_i = (int)nodes[i].vruntime;
    int vruntime_f = (int)((nodes[i].vruntime-vruntime_i)*10000);
    printf(1, "Node %d: PID %d, VRuntime %d.%d\n", i, nodes[i].pid, vruntime_i, vruntime_f);
    if(nodes[i-1].vruntime > nodes[i].vruntime){
      sorted = 0;
    }
  }

  if(sorted && balanced){
    printf(1, "Test Passed: Red-Black Tree maintains BST property\n");
  } else {
    printf(1, "Test Failed: Red-Black Tree does not maintain BST property\n");
  }

  // Additional checks can be added to verify red-black properties

  // Clean up child processes
  for(i = 0; i < 30; i++)
    wait();

  printf(1, "Test completed\n");
  exit();
}
