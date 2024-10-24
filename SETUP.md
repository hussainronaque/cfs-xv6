# Setup Notes

- [Setup Notes](#setup-notes)
  - [The xv6 Source Code](#the-xv6-source-code)
  - [Linux and WSL (Windows Subsystem for Linux)](#linux-and-wsl-windows-subsystem-for-linux)
    - [1. Installing `qemu`](#1-installing-qemu)
    - [2. Installing `gcc` and `gdb`](#2-installing-gcc-and-gdb)
    - [3. Building and Running xv6](#3-building-and-running-xv6)
  - [macOS Build Environment for xv6](#macos-build-environment-for-xv6)
    - [1. Installing `qemu`](#1-installing-qemu-1)
    - [2. Installing the `gcc` Toolchain](#2-installing-the-gcc-toolchain)
    - [3. Building and Running xv6](#3-building-and-running-xv6-1)
  - [Setting Up and Running xv6 Using Docker on macOS 15](#setting-up-and-running-xv6-using-docker-on-macos-15)
    - [Introduction](#introduction)
    - [Pre-requisites](#pre-requisites)
    - [Steps](#steps)
      - [Step 1: Create a Dockerfile](#step-1-create-a-dockerfile)
      - [Step 2: Build the Docker Image](#step-2-build-the-docker-image)
      - [Step 3: Run the Docker Container](#step-3-run-the-docker-container)
      - [Step 4: Build and Run xv6 Inside the Container](#step-4-build-and-run-xv6-inside-the-container)
      - [Step 5: Interact with xv6](#step-5-interact-with-xv6)
      - [Step 6: Exit xv6 and the Docker Container](#step-6-exit-xv6-and-the-docker-container)
      - [Step 7: Restart the Docker Container Later](#step-7-restart-the-docker-container-later)
      - [Step 8: Continue Working with xv6](#step-8-continue-working-with-xv6)
    - [Additional Tips](#additional-tips)
      - [Persisting Your Work](#persisting-your-work)
      - [Exiting and Resuming Workflow Summary](#exiting-and-resuming-workflow-summary)
    - [Troubleshooting](#troubleshooting)
      - [If the xv6 Shell is Unresponsive](#if-the-xv6-shell-is-unresponsive)
      - [Common Issues](#common-issues)
  - [Running Tests](#running-tests)
    - [Important Note](#important-note)
    - [Running Tests in xv6](#running-tests-in-xv6)
    - [Implementing `setnice` System Call](#implementing-setnice-system-call)

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

If everything is set up correctly, xv6 will boot in the terminal. Use `Ctrl-a x` to quit the emulator.

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

Press `Ctrl-a x` to quit the emulator.

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

If everything is installed correctly, you should see xv6 boot up in the terminal. To exit the emulator, press `Ctrl-a x`.

To simplify future builds, edit the `Makefile`:

- Uncomment and modify the `TOOLPREFIX` line as follows:

  ```sh
  TOOLPREFIX = i386-elf-
  ```

  This allows you to run `make qemu-nox` directly.

Now you're ready to explore xv6 on macOS!

## Setting Up and Running xv6 Using Docker on macOS 15

> This guide is prepared and provided by @msaadg.

### Introduction

This guide provides step-by-step instructions for setting up the xv6 operating system on macOS 15 using Docker. It includes creating a Dockerfile, building a Docker image, running the container, building xv6 inside the container, and managing the container lifecycle.

### Pre-requisites

- An Apple Mac running macOS 15 (Sequoia).
- Docker Desktop for Mac installed ([Download here](https://docs.docker.com/desktop/install/mac-install/)).
- xv6 source code placed in a directory on your Mac.

### Steps

#### Step 1: Create a Dockerfile

In your homework project directory, create a file named Dockerfile with the following content:

```Dockerfile
# Use Ubuntu 22.04 base image for amd64 architecture
FROM --platform=linux/amd64 ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Update and install necessary packages
RUN dpkg --add-architecture i386 && \
   apt-get update && \
   apt-get install -y \
   build-essential \
   gcc-multilib \
   gdb-multiarch \
   qemu-system-x86 \
   git \
   make \
   libtool \
   pkg-config

# Create a non-root user (recommended)
RUN useradd -ms /bin/bash xv6user

# Set the working directory
WORKDIR /home/xv6user/xv6

# Change to the non-root user
USER xv6user

# Set the shell
SHELL ["/bin/bash", "-c"]

# Default command
CMD ["/bin/bash"]
```

#### Step 2: Build the Docker Image

Open your terminal, navigate to your homework directory, and run the following command to build the Docker image:

```bash
docker build -t xv6-env .
```

- `-t xv6-env`: Tags the image with the name `xv6-env`.
- `.`: Uses the current directory as the build context.

#### Step 3: Run the Docker Container

Run the Docker container using the following command:

```bash
docker run -it \
   -v "$(pwd)":/home/xv6user/xv6 \
   -w /home/xv6user/xv6 \
   --name xv6-container \
   xv6-env
```

- `-it`: Runs the container in interactive mode with a pseudo-TTY.
- `-v "$(pwd)":/home/xv6user/xv6`: Mounts your current directory into the container.
- `-w /home/xv6user/xv6`: Sets the working directory inside the container.
- `--name xv6-container`: Names the container for easy reference.
- `xv6-env`: Specifies the image to use.

#### Step 4: Build and Run xv6 Inside the Container

Once inside the container, you can build and run xv6:

```bash
cd xv6-public
make clean
make qemu-nox
```

> [!NOTE]
>
> - Ensure your `Makefile` has `TOOLPREFIX` set appropriately or commented out (This is already done by OS staff, just don’t change anything).
> - The `make qemu-nox` command runs xv6 in the terminal without a graphical window.

#### Step 5: Interact with xv6

After running `make qemu-nox`, xv6 will boot, and you can interact with it:

```bash
# In xv6 shell (no prompt is displayed)
ls
```

You should see a list of files similar to:

```bash
.             1 1 512
..            1 1 512
README        2 2 2286
cat           2 3 10984
echo          2 4 10304
...
```

#### Step 6: Exit xv6 and the Docker Container

To exit xv6 and stop the QEMU emulator:

- To exit xv6, press `Ctrl+a` followed by `x`.
- Alternatively, press `Ctrl+a`, then `c`, type quit, and press Enter.

After exiting xv6, exit the Docker container shell:

```bash
exit
```

#### Step 7: Restart the Docker Container Later

When you want to continue working, you can restart the Docker container:

```bash
docker start -ai xv6-container
```

- docker start: Starts a stopped container.
- `-a`: Attaches STDOUT/STDERR and forwards signals.
- `-i`: Keeps STDIN open for interaction.
- `xv6-container`: The name of your container.

#### Step 8: Continue Working with xv6

After restarting the container:

1. Verify you’re in the correct directory:

   ```bash
   pwd
   # Expected output: /home/xv6user/xv6
   ```

2. Build and run xv6 again:

   ```bash
   cd xv6-public
   make clean
   make qemu-nox
   ```

3. Continue your development work as needed.

### Additional Tips

#### Persisting Your Work

Since your xv6 project directory is mounted into the container, all changes are saved on your host machine. You can exit and restart the container without losing your work

#### Exiting and Resuming Workflow Summary

1. **To Exit xv6**: Press `Ctrl+a` then `x`.
2. **To Exit the Docker Container**: Type exit in the container shell.
3. **To Restart the Container**: Run `docker start -ai xv6-container` on your host machine.

### Troubleshooting

#### If the xv6 Shell is Unresponsive

- Ensure the terminal is focused and accepting input.
- Press Enter to see if the shell responds.
- If necessary, exit and restart xv6.

#### Common Issues

- Compilation Errors: Check for typos and ensure all dependencies are installed.
- Docker Permission Errors: Ensure Docker Desktop is running and you have the necessary permissions.

---

## Running Tests

You are **not expected to write new tests**, but we encourage you to explore and play around with the existing ones to better understand the kernel's behavior, You'll need to work with at least `test_scheduler` to make sure your implementation is working.

Playing around with the tests will help you understand how the scheduler behaves under different conditions, and give you insight into system calls and kernel behavior.

#### Important Note

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
   test_scheduler
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
