#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_gettreeinfo(void)
{
  int count;
  int total_weight;
  int period;
  int user_count, user_total_weight, user_period;

  if(argptr(0, (void*)&user_count, sizeof(int)) < 0)
    return -1;
  if(argptr(1, (void*)&user_total_weight, sizeof(int)) < 0)
    return -1;
  if(argptr(2, (void*)&user_period, sizeof(int)) < 0)
    return -1;

  gettreeinfo(&count, &total_weight, &period);

  if(copyout(myproc()->pgdir, user_count, (char*)&count, sizeof(int)) < 0)
    return -1;
  if(copyout(myproc()->pgdir, user_total_weight, (char*)&total_weight, sizeof(int)) < 0)
    return -1;
  if(copyout(myproc()->pgdir, user_period, (char*)&period, sizeof(int)) < 0)
    return -1;

  return 0;
}

int
sys_getprocinfo(void)
{
  int pid;
  struct proc_info *user_pinfo;
  struct proc_info pinfo;

  if(argint(0, &pid) < 0)
    return -1;
  if(argptr(1, (char**)&user_pinfo, sizeof(struct proc_info)) < 0)
    return -1;

  getprocinfo(pid, &pinfo);

  if (pinfo.pid == -1)
    return -1;

  if(copyout(myproc()->pgdir, (uint)user_pinfo, (void*)&pinfo, sizeof(struct proc_info)) < 0)
    return -1;
  return 0;
}

int collect_rb_tree_nodes(struct proc *node, struct rb_node_info *nodes, int *index, int max_nodes);

int
sys_gettreenodes(void)
{
  int max_nodes;
  char *buf;
  struct rbtree *tree = gettree();

  if(argint(0, &max_nodes) < 0)
    return -1;
  if(argptr(1, &buf, max_nodes * sizeof(struct rb_node_info)) < 0)
    return -1;

  struct rb_node_info *nodes = (struct rb_node_info*)kalloc();
  if(nodes == 0)
    return -1;

  int node_index = 0;

  collect_rb_tree_nodes(tree->root, nodes, &node_index, max_nodes);


  if(copyout(myproc()->pgdir, (uint)buf, (void*)nodes, node_index * sizeof(struct rb_node_info)) < 0){
    kfree((char*)nodes);
    return -1;
  }

  kfree((char*)nodes);
  return node_index;  // Return the number of nodes traversed
}

int
collect_rb_tree_nodes(struct proc *node, struct rb_node_info *nodes, int *index, int max_nodes)
{
  if(node == 0 || *index >= max_nodes)
    return 0;

  collect_rb_tree_nodes(node->l, nodes, index, max_nodes);

  nodes[*index].pid = node->pid;
  nodes[*index].vruntime = node->vruntime;
  nodes[*index].color = (node->color == RED) ? 0 : 1;
  nodes[*index].left_pid = (node->l) ? node->l->pid : -1;
  nodes[*index].right_pid = (node->r) ? node->r->pid : -1;
  nodes[*index].parent_pid = (node->p) ? node->p->pid : -1;

  (*index)++;

  collect_rb_tree_nodes(node->r, nodes, index, max_nodes);

  return 0;
}

int
sys_treebalanced(void)
{
  int is_balanced = treebalanced();
  return is_balanced;
}
