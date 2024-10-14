# An xv6 Complete Fair Scheduler (CFS)

In this assignment, you will implement a **Completely Fair Scheduler (CFS)** into the xv6 kernel. This assignment is designed to introduce you to key concepts of kernel development, process scheduling, and system calls. The CFS is a widely used scheduling algorithm in modern operating systems, which ensures that CPU time is fairly distributed among processes. By completing this assignment, you will gain hands-on experience in modifying a real kernel, writing system calls, and evaluating the performance of a new scheduler.

### Objectives
- Develop a deep understanding of xv6 and kernel internals.
- Learn the basics of scheduling algorithms, focusing on CFS.
- Modify the existing scheduler in xv6 and implement a more complex scheduler (CFS).
- Implement and test system calls within the kernel.

### Key Deliverables
- **Scheduler Implementation**: You will implement a CFS scheduler in xv6.
- **System Call Implementation**: You will implement the `setnice()` system call, which adjusts the priority of processes.

### Getting Started
The `xv6-public` provided to you is a fork of https://github.com/mit-pdos/xv6-public with additional system calls. You will primarily be working in [`xv6-public/proc.c`](./xv6-public/proc.c) to implement the scheduler. For your convenience, you are provided with a skeleton of the `rb_tree` functions in [`proc.c`](./proc.c) in the base folder. Do NOT change the name of variables and functions provided to you.

Please refer to the **[SETUP.md](./SETUP.md)** file for instructions on setting up xv6 on your local machine and running tests. You must have a working setup of xv6 before starting the assignment.

## The Completely Fair Scheduler (CFS)

The Completely Fair Scheduler is designed to divide CPU time fairly among all runnable processes. Instead of assigning fixed time slices as in Round-Robin scheduling, CFS keeps track of how much time each process has consumed and prioritizes those that have consumed less time. In practice, this results in a more balanced and "fair" distribution of CPU time, especially when there are processes of varying priorities (nice values).

### Details

1. **System Call: `setnice(int nice_value)`**:
   - This system call allows a process to adjust its **nice value**, which directly affects its weight and scheduling priority.
   - By default, all processes have a nice value of 0. Lowering the nice value (more negative) increases the process's priority, while raising it (more positive) decreases it.
   - You will implement the `setnice()` system call, which should:
     - Set the nice value for the calling process.
     - Adjust the process's weight based on the nice value (use a simplified version of Linux’s formula for weights).
     - Ensure proper integration with the CFS scheduler.
   - The `setnice()` system call signature:
     ```c
     int setnice(int pid, int nice_value);
     ```
   - To add the system call, you will have to modify `syscall.c`, `syscall.h`, `sysproc.c`, `user.h`, and `usys.S`. You can see how other system calls are implemented defined in [`proc.c`](./xv6-public/proc.c) (gettreenodes, getprocinfo, etc.).

2. **Modify the Scheduler**:
   - You will replace the default **Round-Robin** scheduler in xv6 with a simplified version of the **Completely Fair Scheduler (CFS)**.
   - The CFS will maintain a red-black tree where processes are ordered based on their **virtual runtime** (vRuntime), which tracks the amount of time a process has been allowed to execute.

You can follow **[GUIDE.md](./GUIDE.md)** to get a head start on scheduling in xv6 and implementation details for CFS.

### Tests and Grade of the Assignment

In the xv6-public folder all C files that begin with the name of `test` are test scripts. The **[SETUP.md](./SETUP.md)** guide explains how to perform these tests to verify the functionality of your code. These tests will test, among other things, whether weights were assigned to each process correctly, whether RB Tree was initialized correctly, whether the tree maintains red-black properties, whether it maintains balancing properties, whether `vruntime` is assigned correctly to a new process and a recently woken up process, whether higher priority tasks are given more CPU time, whether low-priority tasks are not starving, whether the response time is within scheduling latency, whether CPU times are distributed to processes in proportion to their weights, whether equal-weight processes are treated fairly, whether the scheduler is able to handle a large number of processes, whether scheduler handles processes with different start and end times properly etc. 

`test_all.sh` script runs all the tests as explained in SETUP.md. The same script will be automatically executed by git when you will push your commits to the repo. Your grade will be based on how many of these tests you pass successfully. Each test is assigned a score of 10 marks.
