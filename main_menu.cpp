#include <thread>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include "shell.h"
#include "scheduler.h"
#include "threads.h"
#include "filesystem.h"
#include "memory.h"

using namespace std;

const string RESET   = "\x1b[0m";   // Reset
const string BOLD    = "\x1b[1m";   // Bold
const string CYAN    = "\x1b[36m";  // Cyan
const string YELLOW  = "\x1b[33m";  // Yellow
const string GREEN   = "\x1b[32m";  // Green
const string MAGENTA = "\x1b[35m";  // Magenta
const string BLUE    = "\x1b[34m";  // Blue

void displayWelcomeBox() {
    const string pad = "      ";
    // Total inner width between '|' and '|' is exactly 66 characters
    string lines[] = {
        pad + CYAN + "+------------------------------------------------------------------+" + RESET,
        pad + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "            " + BOLD + YELLOW + "POSIX OPERATING SYSTEMS TOOLKIT SUITE" + RESET + "                 " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "               " + GREEN + "Systems Programming & Lab Suite" + RESET + "                    " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "      * " + BLUE + "POSIX UNIX Terminal Shell (fork, exec, pipe, dup2)" + RESET + "        " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "      * " + BLUE + "CPU Process Scheduler (FCFS, SJF, RR, Priority)" + RESET + "           " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "      * " + BLUE + "Multi-Threading & Concurrency Primitives" + RESET + "                  " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "      * " + BLUE + "Inode Virtual File System Simulation" + RESET + "                      " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "      * " + BLUE + "Virtual Memory & Page Table Management" + RESET + "                    " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "             " + BOLD + MAGENTA + "Systems Architecture & Low-Level C++" + RESET + "                 " + CYAN + "|" + RESET,
        pad + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        pad + CYAN + "+------------------------------------------------------------------+" + RESET
    };

    for (const string& line : lines) {
        cout << line << endl;
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void show_menu() {
    cout << "\n" << BOLD << YELLOW << "===== OS Lab Toolkit Menu =====" << RESET << endl;
    cout << CYAN << "1. Launch Shell" << RESET << endl;
    cout << CYAN << "2. CPU Scheduler" << RESET << endl;
    cout << CYAN << "3. Threading & Synchronization" << RESET << endl;
    cout << CYAN << "4. File System Simulation" << RESET << endl;
    cout << CYAN << "5. Memory Management" << RESET << endl;
    cout << CYAN << "0. Exit" << RESET << endl;
    cout << GREEN << "Enter your choice: " << RESET;
}

int main() {
    int choice;
    displayWelcomeBox();
    while (true) {
        show_menu();
        if (!(cin >> choice)) break;
        system("clear");
        switch (choice) {
            case 1:
                run_shell();
                break;
            case 2:
                launch_scheduler();
                break;
            case 3:
                launch_threads();
                break;
            case 4:
                launch_filesystem();
                break;
            case 5:
                launch_memory();
                break;
            case 0:
                cout << "Exiting... Goodbye!\n";
                return 0;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
    return 0;
}
