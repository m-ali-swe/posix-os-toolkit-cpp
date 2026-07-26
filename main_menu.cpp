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

const string RESET = "\x1b[0m";    // Reset to default
const string BOLD = "\x1b[1m";     // Bold/Bright
const string CYAN = "\x1b[36m";    // Cyan foreground
const string YELLOW = "\x1b[33m";  // Yellow foreground
const string GREEN = "\x1b[32m";   // Green foreground
const string MAGENTA = "\x1b[35m"; // Magenta foreground
const string BLUE = "\x1b[34m";   // Blue foreground

void displayWelcomeBox() {
    const string padding = "      ";
    string lines[] = {
        padding + CYAN + "+------------------------------------------------------------------+" + RESET,
        padding + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "           " + BOLD + YELLOW + "OS LAB TOOLKIT PROJECT - SPRING 2025" + RESET + "                   " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "             " + GREEN + "Developed with passion by:" + RESET + "                           " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "      1. Ahmad Shayan        -  " + BLUE + "CPU Scheduler" + RESET + "                     " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "      2. Muhammad Tayyab     -  " + BLUE + "Threads & Synchronization" + RESET + "         " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "      3. Muhammad Zohaib     -  " + BLUE + "File System Simulation" + RESET + "            " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "      4. Zain Ijaz           -  " + BLUE + "Memory Management" + RESET + "                 " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "      5. Muhammad Ali        -  " + BLUE + "Shell Terminal Interface" + RESET + "          " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "             " + BOLD + MAGENTA + "Let's build the kernel of tomorrow!" + RESET + "                  " + CYAN + "|" + RESET,
        padding + CYAN + "|" + RESET + "                                                                  " + CYAN + "|" + RESET,
        padding + CYAN + "+------------------------------------------------------------------+" + RESET
    };

    for (const string& line : lines) {
        cout << line << endl;
        this_thread::sleep_for(chrono::milliseconds(100)); // animate line-by-line
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
    cout << GREEN << "Enter your choice: " << RESET; // Prompt in green
}

int main() {
    int choice;
    displayWelcomeBox();
    while (true) {
        show_menu();
        cin >> choice;
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
}
