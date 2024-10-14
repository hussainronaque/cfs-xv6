# Setup Notes

To get xv6 up and running on your machine, you'll need to install a few tools. Below, we'll guide you through the setup for macOS, Linux, and Windows Subsystem for Linux (WSL).

## The xv6 Source Code

You have the xv6 source code ready to build, first cd into `xv6-public`

```sh
cd xv6-public
```

## Linux and WSL (Windows Subsystem for Linux)

### 1. Installing `qemu`

On Linux or WSL, the process is similar. Use your package manager to install `qemu` and the necessary toolchain. For example, on Debian-based systems (including Ubuntu and WSL), you can run:

```sh
sudo apt update
sudo apt install qemu-system-x86 gcc-multilib make qemu
```

Some Linux distros won't have `qemu` available. You can try installing the following packages instead:

```sh
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virt-manager gcc-multilib make
```

### 2. Installing `gcc` and `gdb`

To install the required compilers on Linux/WSL, use:

```sh
sudo apt install gcc gdb
```

### 3. Building and Running xv6

Once the tools are installed, navigate to its directory:

```sh
cd xv6-public
```

Run the following command to build and boot xv6:

```sh
make qemu-nox
```

If everything is set up correctly, xv6 will boot in the terminal. Use `C-a x` to quit the emulator.

Now you can build and run xv6 easily with `make qemu-nox`.

## macOS Build Environment for xv6

To run xv6 on macOS, you'll need two main tools: `qemu` (a machine emulator) and a `gcc` cross-compilation toolchain. Here's how to install them:

### 1. Installing `qemu`

`qemu` emulates an x86 system, allowing you to boot and run xv6 without using a real machine. To install `qemu` on macOS, use MacPorts:

```sh
sudo port install qemu
```

This assumes that you have MacPorts installed. If you haven't already installed it, follow the instructions at the [MacPorts install page](https://www.macports.org/install.php). Ensure that `/opt/local/bin` (the typical MacPorts directory) is in your system's `PATH`.

Once installed, test if `qemu` is working by running:

```sh
qemu-system-x86_64 -nographic
```

Press `C-a x` to quit the emulator.

### 2. Installing the `gcc` Toolchain

Install the cross-compilation toolchain to compile xv6:

```sh
sudo port install i386-elf-gcc gdb
```

### 3. Building and Running xv6

Now, in the `xv6-public` directory, run:

```sh
make TOOLPREFIX=i386-elf- qemu-nox
```

If everything is installed correctly, you should see xv6 boot up in the terminal. To exit the emulator, press `C-a x`.

To simplify future builds, edit the `Makefile`:
- Uncomment and modify the `TOOLPREFIX` line as follows:

  ```sh
  TOOLPREFIX = i386-elf-
  ```

  This allows you to run `make qemu-nox` directly.

Now you're ready to explore xv6 on macOS!

---

## Running Tests

You are **not expected to write new tests**, but we encourage you to explore and play around with the existing ones to better understand the kernel's behavior, You'll need to work with at least `test_scheduler` to make sure your implementation is working.

Playing around with the tests will help you understand how the scheduler behaves under different conditions, and give you insight into system calls and kernel behavior.

#### Important Note:
You are **strictly forbidden** from pushing any modifications to the test files. Any changes to these files in your submission will result in **severe penalties**. Please work on these locally and do not commit or push changes to any of the test files.

### Running Tests in xv6

1. **Use `make qemu-nox-test`**:
   - Instead of the usual `make qemu-nox`, you will use `make qemu-nox-test` to compile xv6 and enter the xv6 emulator.
   
   Example:
   ```sh
   make qemu-nox-test
   ```

2. **Entering a Test in the Emulator**:
   Once you enter the xv6 emulator, you can run any test by typing its name. For example, to run **test_scheduler.c** simply:

   ```sh
   $ test_scheduler
   ```

   This will execute the `test_scheduler.c` file, allowing you to see how your scheduler is performing.

3. **Running Tests with `test.sh`**:
   You can also run tests using the provided shell script `test.sh`. This allows for easier management of test runs.:

   ```sh
   ./test.sh test_scheduler --keep
   ```

   - **`--keep`**: This is an optional flag that, when provided, will preserve the build files and output from the test run. Without it, the build will be cleaned up after the test.

4. **Running All Tests with `test_all.sh`**:
   If you want to run all tests, you can use the `test_all.sh` script. It runs all the tests in sequence. This script is provided to help you thoroughly test your code before pushing:

   ```sh
   ./test_all.sh
   ```

5. **File Permissions**:
   Make sure to grant execution permissions to these shell files before running them:

   ```sh
   chmod +x test.sh
   chmod +x test_all.sh
   ```

### Implementing `setnice` System Call

**Important**: The tests will not run unless you implement the `setnice` system call correctly. This system call is crucial to the CFS implementation and is used in several tests, including **test_scheduler.c**. Ensure that `setnice` is fully implemented before attempting to run any tests.

