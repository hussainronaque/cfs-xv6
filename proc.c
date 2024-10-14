#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

struct rbtree rbTree;

static struct rbtree *runnable_tasks = &rbTree;

//Set target scheduler latency and minimum granularity constants
//Latency must be multiples of min_granularity
static int latency = NPROC / 2; // Default period of the scheduler
static int min_granularity = 2; // 2 CPU ticks

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

struct rbtree* gettree(void){
  return runnable_tasks;
}

void
gettreeinfo(int *count, int *total_weight, int *period)
{
  *count = runnable_tasks->length;
  *total_weight = runnable_tasks->total_weight;
  *period = runnable_tasks->period;
}

void
getprocinfo(int pid, struct proc_info *info)
{
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      info->pid = p->pid;
      info->nice_value = p->nice_value;
      info->weight = p->weight;
      info->vruntime = p->vruntime;
      info->curr_runtime = p->curr_runtime;
      release(&ptable.lock);
      return;
    }
  }
  release(&ptable.lock);
  info->pid = -1;
}


int check_rb_tree_properties(struct proc *node, int black_count, int *path_black_count);

int
treebalanced(void)
{
  int is_balanced = 1;
  int path_black_count = -1;

  // Check if the root is black (Property 2)
  if(runnable_tasks->root != 0 && runnable_tasks->root->color != BLACK)
    is_balanced = 0;
  else
    is_balanced = check_rb_tree_properties(runnable_tasks->root, 0, &path_black_count);

  return is_balanced;
}

int
check_rb_tree_properties(struct proc *node, int black_count, int *path_black_count)
{
  if(node == 0){
    // Reached a leaf (NIL node), increment black count for NIL nodes
    if(*path_black_count == -1){
      *path_black_count = black_count;
    } else if(black_count != *path_black_count){
      // Property 5 violated
      return 0;
    }
    return 1;
  }

  // Check Property 4: If a node is red, then both its children must be black
  if(node->color == RED){
    if((node->l != 0 && node->l->color == RED) ||
       (node->r != 0 && node->r->color == RED)){
      // Property 4 violated
      return 0;
    }
  }

  // Increment black count if the node is black
  if(node->color == BLACK)
    black_count++;

  // Recursively check left and right subtrees
  if(!check_rb_tree_properties(node->l, black_count, path_black_count))
    return 0;
  if(!check_rb_tree_properties(node->r, black_count, path_black_count))
    return 0;

  return 1;
}

// compute_weight(int nice_value)
// This function should compute the weight of a process based on its nice value.
// Use the formula: weight = 1024 / (1.25 ^ nice_value).
// Make sure the nice_value is clamped between -20 and 19.
int
compute_weight(int nice_value)
{
}

// setnice(int pid, int nice_value)
// This function sets the nice value for a process identified by its PID.
// Recalculate the weight of the process after setting the nice value.
// Ensure that the nice_value is within valid bounds (-20 to 19) and clamp if needed.
int
setnice(int pid, int nice_value)
{
}

// treeinit(struct rbtree *tree, char *lockName)
// Initializes the red-black tree for the runnable processes. 
// Set the tree's root to 0 and initialize its lock, count, and total weight.
void
treeinit(struct rbtree *tree, char *lockName)
{
}

// full(struct rbtree *tree)
// Returns 1 if the red-black tree is full (i.e., has reached the maximum number of processes NPROC), otherwise 0.
int
full(struct rbtree *tree)
{
}

// leftrotate(struct rbtree *tree, struct proc* p)
// Performs a left rotation on the red-black tree starting from the specified process node.
// Maintain the tree's properties during the rotation.
void 
leftrotate(struct rbtree* tree, struct proc* p){
}

// rightrotate(struct rbtree *tree, struct proc* p)
// Performs a right rotation on the red-black tree starting from the specified process node.
// Maintain the tree's properties during the rotation.
void 
rightrotate(struct rbtree* tree, struct proc* p){
}

// minproc(struct proc* p)
// Traverses the tree to find the process with the minimum virtual runtime (vruntime).
// Returns a pointer to this process.
struct proc*
minproc(struct proc* p){
}

// insertproc(struct proc* trav, struct proc* p)
// Inserts a new process into the red-black tree, preserving the tree's properties.
// Ensure that the process is inserted in the correct location based on its vruntime.
struct proc*
insertproc(struct proc* trav, struct proc* p){
}

// deleteproc(struct proc* trav, struct proc* p)
// Removes a specified process from the red-black tree, preserving the tree's properties.
struct proc*
deleteproc(struct rbtree* tree, struct proc* p){
}

// fixinsert(struct rbtree* tree, struct proc* p)
// Fixes any violations of red-black tree properties after a new process is inserted.
// Implement different cases to restore the properties (e.g., color adjustments, rotations).
void
fixinsert(struct rbtree* tree, struct proc* p)
{
}

// add_to_tree(struct rbtree* tree, struct proc* p)
// Adds a process to the red-black tree and ensures that the tree properties are maintained.
// Recalculate the tree's total weight and find the new minimum vruntime.
void
add_to_tree(struct rbtree* tree, struct proc* p){
}

// fixdelete(struct rbtree* tree, struct proc* parentProc, struct proc* p)
// Fixes any violations of red-black tree properties after a process is deleted.
// Handle different cases that arise after the deletion, such as rebalancing the tree.
// Note: We'll only need to delete specific node from the tree so all cases need not be handled.
void
fixdelete(struct rbtree* tree, struct proc* parentProc, struct proc* p){
}

// next_process(struct rbtree* tree)
// Retrieves the process with the smallest vruntime from the tree.
// Remove the process from the tree and adjust the tree's total weight.
struct proc*
next_process(struct rbtree* tree){
}

// should_preempt(struct proc* current, struct proc* min_vruntime)
// Checks if the current process should be preempted based on its vruntime and execution time.
// Preemption occurs if the current process exceeds its time slice or if another process has a smaller vruntime.
int
should_preempt(struct proc* current, struct proc* min_vruntime){
}

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  treeinit(runnable_tasks, "runnable_tasks");
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  // Initialize CFS members of the process.
  p->vruntime = 0; // This should be set to minimum vruntime
  p->curr_runtime = 0;
  p->time_slice = 0;
  p->nice_value = 0;
  p->weight = compute_weight(p->nice_value);

  // Initialize red-black tree members of the process
  p->l = 0;
  p->r = 0;
  p->p = 0;
  
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  struct proc *p = myproc();
  acquire(&ptable.lock);  //DOC: yieldlock
  p->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runnable",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
