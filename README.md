# 🐧 POSIX OS Toolkit & UNIX Shell Interface (C++)

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![POSIX Standard](https://img.shields.io/badge/POSIX-1-000000?style=for-the-badge&logo=linux)](https://pubs.opengroup.org/onlinepubs/9699919799/)
[![GCC / G++](https://img.shields.io/badge/Compiler-GCC_%2F_G%2B%2B-5C6F84?style=for-the-badge&logo=gnu)](https://gcc.gnu.org/)
[![Linux / Unix](https://img.shields.io/badge/Platform-Linux_%2F_POSIX-FCC624?style=for-the-badge&logo=linux)](https://www.kernel.org/)

**POSIX OS Toolkit** is a low-level Systems Programming & Operating Systems suite implemented in C++20. It combines an interactive POSIX-compliant UNIX Command Shell, a CPU Process Scheduling engine, a Multi-Thread Synchronization module (using `std::binary_semaphore`), an Inode Virtual File System, and a Paging Memory Management simulator.

---

## 👥 Core Module Ownership

- 🐚 **POSIX UNIX Shell Interface (`shell.cpp` & `shell.h`)**: Lead Systems Developer — [Muhammad Ali](https://github.com/m-ali-swe)
- ⏱️ **CPU Process Scheduler (`scheduler.cpp`)**: Core Systems Module
- 🧵 **POSIX Threading & Concurrency (`threads.cpp`)**: Core Systems Module
- 📂 **Inode Virtual File System (`filesystem.cpp`)**: Core Systems Module
- 🧠 **Virtual Memory & Page Table Engine (`memory.cpp`)**: Core Systems Module

---

## 🛠️ Toolkit Modules Overview

### 1. 🐚 POSIX UNIX Shell Terminal Interface (`shell.cpp`)
Interactive UNIX command-line interpreter implementing low-level POSIX system call interfaces:
- **Process Creation & Lifecycle Management**: Implements process cloning via `fork()` and executable space replacement via `execvp()` with dynamic C-style argument pointers (`char** argv`). Uses `waitpid()` to collect exit status codes and prevent zombie process accumulation.
- **Inter-Process Pipeline Architecture (`|`)**: Supports multi-stage command chaining (`cmd1 | cmd2 | cmd3`) via `pipe(int pipefd[2])` data channels and `dup2()` file descriptor redirection.
- **File Descriptor I/O Redirection (`<`, `>`, `>>`)**: Manages input reading (`<`), output truncating (`>`), and output appending (`>>`) using POSIX `open()` flags (`O_RDONLY`, `O_WRONLY | O_CREAT | O_TRUNC/O_APPEND`).
- **Asynchronous Background Execution (`&`)**: Trailing `&` execution runs child processes asynchronously in non-blocking background sub-shells.
- **Signal Trapping (`SIGINT`)**: Custom handler for `SIGINT` (Ctrl+C) using `volatile sig_atomic_t` flags to maintain prompt loop responsiveness without crashing.
- **Native Shell Built-ins & Dynamic Prompt**: Built-in `cd` (with `-` path tracking and `~` home resolution), `pwd` (using `getcwd()`), `history`, `echo`, `exit`, and an ANSI-colored prompt (`myShell@System:~/path$`).

---

### 2. ⏱️ CPU Process Scheduler (`scheduler.cpp`)
Simulates process execution queues and evaluates CPU scheduling policy performance:
- **Algorithms Implemented**: First-Come First-Served (FCFS), Shortest Job First (SJF), Round Robin (RR with configurable time quantum), and Preemptive/Non-Preemptive Priority Scheduling.
- **Performance Analysis**: Calculates Average Waiting Time, Turnaround Time, Response Time, and renders ASCII Gantt charts.

---

### 3. 🧵 POSIX Threading & Concurrency (`threads.cpp`)
Demonstrates multi-threaded execution and thread synchronization primitives:
- **Primitives**: `std::thread`, `std::mutex`, `std::condition_variable`, `std::binary_semaphore` (C++20).
- **Classic Problems**: Bounded-buffer Producer-Consumer pattern and Readers-Writers synchronization avoiding race conditions.

---

### 4. 📂 Inode Virtual File System (`filesystem.cpp`)
Simulates UNIX file system storage structures in memory:
- **Data Structures**: Inode allocation tables, directory tree nodes, and virtual data blocks.
- **Operations**: File creation, deletion, directory navigation (`mkdir`, `ls`, `cat`), and block allocation tracking.

---

### 5. 🧠 Virtual Memory & Page Table Engine (`memory.cpp`)
Simulates OS virtual memory address translation and page replacement:
- **Paging Engine**: Maps logical virtual addresses to physical frame numbers using page tables.
- **Page Replacement Algorithms**: First-In First-Out (FIFO), Least Recently Used (LRU), and Optimal Page Replacement simulating page fault rates.

---

## ⚡ POSIX Shell Command Pipeline Sequence

```
User Input: "cat data.txt | grep -i error > error.log &"
                           │
                           ▼
                  [parse_line()]
                           │
         ┌─────────────────┴─────────────────┐
         ▼                                   ▼
 Command 1: "cat data.txt"           Command 2: "grep -i error"
 (Redirect STDIN from data.txt)     (Redirect STDOUT to error.log)
         │                                   │
         ├─────────── pipe() Channel ────────┤
         │                                   │
         ▼                                   ▼
    fork() Child 1                      fork() Child 2
  dup2(pipe[1], STDOUT)               dup2(pipe[0], STDIN)
  execvp("cat", ...)                  execvp("grep", ...)
```

---

## 📁 Repository Structure

```
posix-os-toolkit-cpp/
├── shell.cpp               # POSIX Shell implementation (Fork, Exec, Pipe, Redirection)
├── shell.h                 # Shell header file
├── scheduler.cpp           # CPU Process Scheduling algorithms (FCFS, SJF, RR, Priority)
├── scheduler.h             # Scheduler header file
├── threads.cpp             # Threading & synchronization primitives (Mutex, Semaphore)
├── threads.h               # Threads header file
├── filesystem.cpp          # Virtual Inode File System simulation
├── filesystem.h            # File system header file
├── memory.cpp              # Virtual memory paging & page replacement (FIFO, LRU)
├── memory.h                # Memory manager header file
├── main_menu.cpp           # Animated interactive CLI menu entry point
├── Makefile                # Build compilation script (C++20)
├── .gitignore              # Git ignore rules for build artifacts
└── README.md
```

---

## 🛠️ Compilation & Execution Guide

### Prerequisites
- **Compiler**: `g++` with **C++20** support (`GCC 11+`)
- **OS Environment**: Linux, macOS, or Windows WSL (POSIX environment required for `unistd.h` system calls)

---

### 1. Clone & Build with `g++` (Recommended)

```bash
# Clone the repository
git clone https://github.com/m-ali-swe/posix-os-toolkit-cpp.git
cd posix-os-toolkit-cpp

# Compile all source files into the OSLabToolkit executable
g++ -std=c++20 *.cpp -o OSLabToolkit -pthread

# Run executable
./OSLabToolkit
```

### 2. Alternative Build with `make`

```bash
# Compile using Makefile
make

# Execute OS Toolkit
./OSLabToolkit
```

### 3. Cleaning Build Artifacts

```bash
make clean
```
