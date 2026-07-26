#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <string>   // For std::string and to_string
#include <limits>   // For numeric_limits
#include <cstdlib>  // For system()
using namespace std::chrono_literals;

#define COLOR_RESET     "\033[0m"
#define COLOR_HEADER    "\033[1;34m"
#define COLOR_HIGHLIGHT "\033[1;36m"
#define COLOR_INPUT     "\033[1;32m"
#define COLOR_ERROR     "\033[1;31m"

using namespace std;

// Cross-platform clear screen macro
#ifdef _WIN32
#include <windows.h>
#define clearScreen() system("cls")
#define sleep_ms(x) Sleep(x) // Sleep in milliseconds for Windows
#else
#include <unistd.h>
#define clearScreen() system("clear")
#define sleep_ms(x) usleep(x * 1000) // usleep takes microseconds
#endif


// 👋 Welcome screen with graphics style
void showWelcomeScreen() {
    string welcome = "Welcome to Memory Allocation Algorithms";
    clearScreen(); // Clear screen before welcome
    cout << COLOR_HEADER << "=========================================" << endl;
    for (char c : welcome) {
        cout << c << flush;
        this_thread::sleep_for(40ms);
    }
    cout << "\n=========================================" << endl;
    cout << "          By Zain (💻)\n";
    cout << "    Topics: First Fit, Best Fit, Worst Fit\n";
    cout << "-----------------------------------------\n\n" << COLOR_RESET;
    this_thread::sleep_for(chrono::seconds(2)); // Pause for welcome screen
    clearScreen(); // Clear after welcome
}

// Helper function to get memory block sizes
vector<int> getBlockInput() {
    int nBlocks;
    cout << COLOR_INPUT << "Enter number of memory blocks: " << COLOR_RESET;
    cin >> nBlocks;
    // Clear input buffer after reading integer
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<int> blocks(nBlocks);
    for (int i = 0; i < nBlocks; i++) {
        cout << COLOR_INPUT << "Enter size of Block " << i + 1 << ": " << COLOR_RESET;
        cin >> blocks[i];
        // Clear input buffer after reading integer
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return blocks;
}

// Helper function to get process sizes
vector<int> getProcessInput() {
    int nProcesses;
    cout << COLOR_INPUT << "Enter number of processes: " << COLOR_RESET;
    cin >> nProcesses;
    // Clear input buffer after reading integer
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<int> processes(nProcesses);
    for (int i = 0; i < nProcesses; i++) {
        cout << COLOR_INPUT << "Enter size of Process " << i + 1 << ": " << COLOR_RESET;
        cin >> processes[i];
        // Clear input buffer after reading integer
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return processes;
}


// 🔹 First Fit Algorithm
void firstFit(const vector<int>& blocks, const vector<int>& processes) {
    vector<bool> used(blocks.size(), false);
    vector<int> allocation(processes.size(), -1);

    for (int i = 0; i < processes.size(); i++) {
        for (int j = 0; j < blocks.size(); j++) {
            if (!used[j] && blocks[j] >= processes[i]) {
                allocation[i] = j;
                used[j] = true;
                break;
            }
        }
    }

    cout << "\n--- First Fit Allocation ---\n";
    for (int i = 0; i < processes.size(); i++) {
        if (allocation[i] != -1)
            cout << "Process " << i + 1 << " allocated to Block " << allocation[i] + 1 << endl;
        else
            cout << "Process " << i + 1 << " not allocated.\n";
    }
}

// 🔹 Best Fit Algorithm
void bestFit(const vector<int>& blocks, const vector<int>& processes) {
    vector<bool> used(blocks.size(), false);
    vector<int> allocation(processes.size(), -1);

    for (int i = 0; i < processes.size(); i++) {
        int bestIndex = -1;
        for (int j = 0; j < blocks.size(); j++) {
            if (!used[j] && blocks[j] >= processes[i]) {
                if (bestIndex == -1 || blocks[j] < blocks[bestIndex])
                    bestIndex = j;
            }
        }
        if (bestIndex != -1) {
            allocation[i] = bestIndex;
            used[bestIndex] = true;
        }
    }

    cout << "\n--- Best Fit Allocation ---\n";
    for (int i = 0; i < processes.size(); i++) {
        if (allocation[i] != -1)
            cout << "Process " << i + 1 << " allocated to Block " << allocation[i] + 1 << endl;
        else
            cout << "Process " << i + 1 << " not allocated.\n";
    }
}

// 🔹 Worst Fit Algorithm
void worstFit(const vector<int>& blocks, const vector<int>& processes) {
    vector<bool> used(blocks.size(), false);
    vector<int> allocation(processes.size(), -1);

    for (int i = 0; i < processes.size(); i++) {
        int worstIndex = -1;
        for (int j = 0; j < blocks.size(); j++) {
            if (!used[j] && blocks[j] >= processes[i]) {
                if (worstIndex == -1 || blocks[j] > blocks[worstIndex])
                    worstIndex = j;
            }
        }
        if (worstIndex != -1) {
            allocation[i] = worstIndex;
            used[worstIndex] = true;
        }
    }

    cout << "\n--- Worst Fit Allocation ---\n";
    for (int i = 0; i < processes.size(); i++) {
        if (allocation[i] != -1)
            cout << "Process " << i + 1 << " allocated to Block " << allocation[i] + 1 << endl;
        else
            cout << "Process " << i + 1 << " not allocated.\n";
    }
}

// 🔸 Main Function for Memory Module
void launch_memory() {
    showWelcomeScreen(); // Show welcome graphics

    int choice;
    do {
        cout << COLOR_HEADER << "=========================================\n";
        cout << "     MEMORY ALLOCATION ALGORITHMS\n";
        cout << "=========================================\n";
        cout << "Choose memory allocation algorithm:\n";
        cout << "1. First Fit\n";
        cout << "2. Best Fit\n";
        cout << "3. Worst Fit\n";
        cout << "0. Exit to Main Menu\n"; // Added exit option
        cout << "Enter your choice (0-3): " << COLOR_RESET;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer

        clearScreen(); // Clear screen after choice, before processing or next menu

        if (choice >= 1 && choice <= 3) {
            // Get input for blocks and processes AFTER algorithm choice
            vector<int> blocks = getBlockInput();
            vector<int> processes = getProcessInput();

            // Processing animation
            cout << COLOR_HIGHLIGHT << "\nProcessing";
            for (int i = 0; i < 3; ++i) {
                cout << "." << flush;
                this_thread::sleep_for(300ms);
            }
            cout << "\n" << COLOR_RESET;

            switch (choice) {
                case 1: firstFit(blocks, processes); break;
                case 2: bestFit(blocks, processes); break;
                case 3: worstFit(blocks, processes); break;
            }

            cout << "\n\nPress Enter to return to menu...";
            cin.get(); // Wait for user to press Enter
            clearScreen(); // Clear screen for the next menu display
        } else if (choice == 0) {
            cout << COLOR_HIGHLIGHT << "Exiting Memory Allocation Simulator.\n" << COLOR_RESET;
            this_thread::sleep_for(chrono::seconds(1));
            clearScreen();
            return; // Return to the main toolkit menu
        } else {
            cout << COLOR_ERROR << "Invalid choice! Please enter 0, 1, 2, or 3.\n" << COLOR_RESET;
            this_thread::sleep_for(chrono::seconds(1));
            clearScreen(); // Clear after error message before re-displaying menu
        }

    } while (true); // Loop indefinitely until 'return'
}