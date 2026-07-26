#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <queue>
#include <limits>
#include <climits>
#include <string> // Added for to_string and general string manipulation
#include <tuple>  // For std::tuple

// Conditional compilation for clearing screen and getting character input
#ifdef _WIN32
#include <windows.h> // For system("cls") on Windows and SetConsoleTextAttribute
#include <conio.h>   // For _getch on Windows
#define CLEAR_SCREEN() system("cls")
#define GET_CHAR() _getch()
#else
#define CLEAR_SCREEN() system("clear") // For Linux/macOS
#include <termios.h>                 // For non-blocking getch alternative
#include <unistd.h>                  // For non-blocking getch alternative

// Simple cross-platform getch alternative
char GET_CHAR() {
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}
#endif

using namespace std;

// Function to print colored styled text (Declaration)
void printStyled(string text, string color, bool isBold);

// Function to print welcome screen
void printWelcome() {
    printStyled("\t\t\t==============================================================", "cyan", true);
    printStyled("\t\t\t\t        CPU Scheduling Simulator (C++)      ", "green", true);
    printStyled("\t\t\t==============================================================", "cyan", true);
    cout << "\t\t\t\t\t Created by: Ahmad Shayan\n";
    cout << "\t\t\t\t\t Roll No : Sp-23/BSCS/073 \n";
    cout << "\t\t\t\t It Includes : FCFS, SJF (N & P), LRTF, RR, Priority (N & P)\n";
    printStyled("\t\t=====================================================================================\n", "cyan", true);
}

// Function to print colored styled text (Definition)
void printStyled(string text, string color, bool isBold) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int colorCode = 7; // Default white

    if (color == "red")
        colorCode = 12;
    else if (color == "green")
        colorCode = 10;
    else if (color == "yellow")
        colorCode = 14;
    else if (color == "blue")
        colorCode = 9;
    else if (color == "cyan")
        colorCode = 11;
    else if (color == "magenta")
        colorCode = 13;
    else if (color == "gray")
        colorCode = 8;
    else
        colorCode = 7;

    if (isBold)
        colorCode |= FOREGROUND_INTENSITY;

    SetConsoleTextAttribute(hConsole, colorCode);
    cout << text << endl;
    SetConsoleTextAttribute(hConsole, 7); // Reset to default
#else
    // ANSI escape codes for coloring on Linux/macOS terminals
    string color_code;
    if (color == "red")
        color_code = "\033[31m";
    else if (color == "green")
        color_code = "\033[32m";
    else if (color == "yellow")
        color_code = "\033[33m";
    else if (color == "blue")
        color_code = "\033[34m";
    else if (color == "cyan")
        color_code = "\033[36m";
    else if (color == "magenta")
        color_code = "\033[35m";
    else if (color == "gray")
        color_code = "\033[90m";
    else
        color_code = "\033[0m"; // Reset

    string bold_code = isBold ? "\033[1m" : "";
    string reset_code = "\033[0m";

    cout << bold_code << color_code << text << reset_code << endl;
#endif
}


// Process structure
typedef struct Process {
    int pid, at, bt, ct, tat, wt, rt, remaining_bt, start_time;
    int priority;
    bool visited; // Used in non-preemptive SJF/Priority to mark completed
} Process;

// --- Improved Gantt Chart Logic ---
// Utility function to print Gantt chart
// timeline now stores {start_time, end_time, process_name}
void printGanttChart(vector<tuple<int, int, string>> &timeline) {
    if (timeline.empty()) {
        printStyled("\nGantt Chart: (Empty)\n", "yellow", false);
        return;
    }

    printStyled("\nGantt Chart:", "magenta", true);

    string top_border_line = "+";
    string process_names_line = "|";
    string bottom_border_line = "+";
    string timestamps_line = "";

    vector<int> block_widths;

    // First pass: Determine widths and build lines
    for (const auto &block : timeline) {
        int start_time = get<0>(block);
        int end_time = get<1>(block);
        string name = get<2>(block);
        int duration = end_time - start_time;

        // Calculate minimum width required for this block
        // At least length of name + 2 for padding, or length of end time + 1 for padding (for single digit times)
        // Ensure it's at least 4 characters for "P1  " or "Idle"
        int min_width = max({(int)name.length() + 2, (int)to_string(end_time).length() + 1, 4});

        // Proportional width based on duration. Each unit of time gets 2 chars.
        int calculated_width = max(min_width, duration * 2); 
        
        // Ensure minimum width for time string alignment
        if (duration == 0 && name != "Idle") { // For 0-duration slices (e.g., for preemptive where time increments by 1)
            calculated_width = max(min_width, (int)to_string(end_time).length() + 1);
        } else if (duration == 0 && name == "Idle") {
            // If it's a 0-duration idle, it probably shouldn't be here, but just in case
            calculated_width = max(min_width, (int)to_string(end_time).length() + 1);
        }

        block_widths.push_back(calculated_width);

        // Build top and bottom border lines
        for (int i = 0; i < calculated_width; ++i) {
            top_border_line += "-";
            bottom_border_line += "-";
        }
        top_border_line += "+";
        bottom_border_line += "+";

        // Build process names line
        int padding_left = (calculated_width - name.length()) / 2;
        int padding_right = calculated_width - name.length() - padding_left;
        
        process_names_line += string(padding_left, ' ') + name + string(padding_right, ' ') + "|";
    }

    // Second pass: Build timestamps line
    timestamps_line += to_string(get<0>(timeline[0])); // Start time of the very first block

    for (size_t i = 0; i < timeline.size(); ++i) {
        int end_time = get<1>(timeline[i]);
        string end_time_str = to_string(end_time);

        int current_block_width = block_widths[i];
        
        // Calculate the number of spaces needed before the end_time for alignment
        int spaces_needed = current_block_width - (int)end_time_str.length();
        
        // If it's the first block, we need to account for the starting '0' or initial AT
        if (i == 0) {
            spaces_needed -= to_string(get<0>(timeline[i])).length();
        } else {
            // For subsequent blocks, the previous end time is the current start time
            // So we need to align the end time string within the current block's width
            // relative to where the previous end time string ended.
        }
        
        timestamps_line += string(max(0, spaces_needed), ' ');
        timestamps_line += end_time_str;
    }


    cout << top_border_line << endl;
    cout << process_names_line << endl;
    cout << bottom_border_line << endl;
    cout << timestamps_line << "\n";
}

// Utility function to print the final table
void printTable(vector<Process> &p, bool show_priority = false) {
    printStyled("\nProcess Table:", "magenta", true);
    if (show_priority)
        cout << "PID\tAT\tBT\tPR\tCT\tTAT\tWT\tRT\n";
    else
        cout << "PID\tAT\tBT\tCT\tTAT\tWT\tRT\n";

    float avg_tat = 0, avg_wt = 0, avg_rt = 0;
    for (auto &pr : p) {
        if (show_priority)
            cout << pr.pid << "\t" << pr.at << "\t" << pr.bt << "\t" << pr.priority << "\t";
        else
            cout << pr.pid << "\t" << pr.at << "\t" << pr.bt << "\t";
        cout << pr.ct << "\t" << pr.tat << "\t" << pr.wt << "\t" << pr.rt << "\n";
        avg_tat += pr.tat;
        avg_wt += pr.wt;
        avg_rt += pr.rt;
    }
    int n = p.size();
    cout << fixed << setprecision(2);
    printStyled("Average TAT: " + to_string(avg_tat / n) + ", WT: " + to_string(avg_wt / n) + ", RT: " + to_string(avg_rt / n), "yellow", false);
    cout << "\n";
}

// Sort utility based on arrival time, burst time, priority (for initial sorting, not runtime selection)
bool sortByAT(const Process &a, const Process &b) {
    return (a.at < b.at || (a.at == b.at && a.pid < b.pid));
}


// Function to get process details from user
vector<Process> getProcessInput(bool needsPriority = false) {
    int n;
    cout << "Enter the number of processes: ";
    cin >> n;
    vector<Process> processes(n);
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        cout << "Enter arrival time for P" << processes[i].pid << ": ";
        cin >> processes[i].at;
        cout << "Enter burst time for P" << processes[i].pid << ": ";
        cin >> processes[i].bt;
        if (needsPriority) {
            cout << "Enter priority for P" << processes[i].pid << " (Lower Number = Higher Priority) : ";
            cin >> processes[i].priority;
        }
        processes[i].remaining_bt = processes[i].bt;
        processes[i].visited = false;
        processes[i].start_time = -1; // Initialize start_time
    }
    return processes;
}

// -------------  First Come First Serve (FCFS) Start --------------
void FCFS(vector<Process> p) {
    sort(p.begin(), p.end(), sortByAT);
    int t = 0;
    vector<tuple<int, int, string>> timeline_gantt;

    for (auto &pr : p) {
        if (t < pr.at) {
            timeline_gantt.emplace_back(t, pr.at, "Idle");
            t = pr.at; // Advance time to process arrival
        }
        pr.start_time = t;
        pr.ct = t + pr.bt;
        pr.tat = pr.ct - pr.at;
        pr.wt = pr.start_time - pr.at;
        pr.rt = pr.start_time - pr.at;

        timeline_gantt.emplace_back(t, pr.ct, "P" + to_string(pr.pid));
        t = pr.ct; // Advance time to process completion
    }
    printGanttChart(timeline_gantt);
    printTable(p);
}
// -----------------  First Come First Serve (FCFS) End ---------------

// ------------ Shortest Job First (Non-Preemptive) Start -------------
void SJF_NonPreemptive(vector<Process> p) {
    int n = p.size(), t = 0, completed = 0;
    vector<tuple<int, int, string>> timeline_gantt;
    for (auto &pr : p)
        pr.visited = false; // Reset visited flag for each run

    int current_block_start_time = 0; // For Gantt chart

    while (completed < n) {
        int idx = -1;
        int min_bt = INT_MAX; // Shortest Burst Time
        int min_at = INT_MAX; // Earliest Arrival Time (for tie-breaking)

        for (int i = 0; i < n; i++) {
            if (!p[i].visited && p[i].at <= t) {
                // Tie-breaking: smaller BT, then smaller AT, then smaller PID
                if (p[i].bt < min_bt) {
                    min_bt = p[i].bt;
                    min_at = p[i].at;
                    idx = i;
                } else if (p[i].bt == min_bt) {
                    if (p[i].at < min_at) {
                        min_at = p[i].at;
                        idx = i;
                    } else if (p[i].at == min_at && p[i].pid < p[idx].pid) {
                        idx = i;
                    }
                }
            }
        }
        if (idx == -1) {
            // CPU is idle.
            if (timeline_gantt.empty() || get<2>(timeline_gantt.back()) != "Idle" || get<1>(timeline_gantt.back()) != t) {
                if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) != "Idle" && get<1>(timeline_gantt.back()) == t) {
                    // This means a process just finished at time `t`, and now CPU is idle from `t`
                    // So we must start a new idle block from `t`.
                }
                timeline_gantt.emplace_back(t, t + 1, "Idle");
            } else {
                get<1>(timeline_gantt.back())++; // Extend last idle block
            }
            t++;
            current_block_start_time = t; // New potential block start if idle extends
        } else {
            // Process selected. If there was an idle block, close it.
            if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) > t) {
                get<1>(timeline_gantt.back()) = t; // Trim the idle block
            } else if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<0>(timeline_gantt.back()) == t) {
                 timeline_gantt.pop_back(); // Remove 0-duration idle block
            }

            p[idx].start_time = t;
            p[idx].ct = t + p[idx].bt;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].start_time - p[idx].at;
            p[idx].rt = p[idx].wt; // For non-preemptive, response time = waiting time
            p[idx].visited = true;

            timeline_gantt.emplace_back(t, p[idx].ct, "P" + to_string(p[idx].pid));
            t = p[idx].ct;
            completed++;
            current_block_start_time = t; // New potential block start after process finishes
        }
    }
    printGanttChart(timeline_gantt);
    printTable(p);
}
//-------------- Shortest Job First (Non-Preemptive) End ----------------------

// ------------------ Shortest Job First (Preemptive) Start ---------------
void SJF_Preemptive(vector<Process> p) {
    int n = p.size(), t = 0, completed = 0;
    int prev_idx = -1; // -1: no process, valid_idx: process Px, -2: idle
    for (auto &pr : p)
        pr.remaining_bt = pr.bt;
    vector<tuple<int, int, string>> timeline_gantt;

    int current_block_start_time = 0;

    while (completed < n) {
        int idx = -1;
        int min_rem_bt = INT_MAX; // Shortest Remaining Burst Time
        int min_at = INT_MAX; // Earliest Arrival Time (for tie-breaking)

        for (int i = 0; i < n; i++) {
            if (p[i].at <= t && p[i].remaining_bt > 0) {
                // Tie-breaking: smaller Remaining BT, then smaller AT, then smaller PID
                if (p[i].remaining_bt < min_rem_bt) {
                    min_rem_bt = p[i].remaining_bt;
                    min_at = p[i].at;
                    idx = i;
                } else if (p[i].remaining_bt == min_rem_bt) {
                    if (p[i].at < min_at) {
                        min_at = p[i].at;
                        idx = i;
                    } else if (p[i].at == min_at && p[i].pid < p[idx].pid) {
                        idx = i;
                    }
                }
            }
        }

        if (idx == -1) { // CPU is Idle
            if (prev_idx != -2) { // If previous state was not idle, close the previous block
                if (prev_idx != -1) { // If it was a process that just got preempted or finished at `t`
                    timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
                }
                current_block_start_time = t; // Start of new idle block
            }
            if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) == t) {
                get<1>(timeline_gantt.back())++; // Extend last idle block
            } else {
                 timeline_gantt.emplace_back(t, t+1, "Idle");
            }
            prev_idx = -2; // Mark as idle state
            t++;
            continue;
        }

        // A process is selected (idx is valid)
        if (prev_idx != idx) { // Context switch occurred (or coming from idle)
            if (prev_idx != -1) { // If there was a previous block
                if (prev_idx == -2) { // If previous was idle, make sure its duration is correct
                     if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<0>(timeline_gantt.back()) == current_block_start_time) {
                         get<1>(timeline_gantt.back()) = t;
                     }
                } else { // If previous was a process, close its block
                    timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
                }
            }
            current_block_start_time = t; // Start of new process block
            prev_idx = idx; // Update previous process
        }

        if (p[idx].remaining_bt == p[idx].bt) { // If this is the very first time this process runs
            p[idx].start_time = t; // Record first start time
        }

        p[idx].remaining_bt--;
        t++;

        if (p[idx].remaining_bt == 0) {
            // Process finished
            p[idx].ct = t;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            p[idx].rt = p[idx].start_time - p[idx].at;
            completed++;

            timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[idx].pid));
            prev_idx = -1; // Reset prev_idx to indicate no active process after completion
            current_block_start_time = t; // Next block starts at `t`
        }
    }
    // After the loop, if the last block wasn't added (e.g., it was an active process that finished the simulation)
    if (prev_idx != -1 && prev_idx != -2 && current_block_start_time < t) {
        timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
    } else if (prev_idx == -2 && current_block_start_time < t) { // If it ended on an idle period
        if(!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) < t) {
            get<1>(timeline_gantt.back()) = t; // Extend last idle block to final time
        }
    }

    printGanttChart(timeline_gantt);
    printTable(p);
}

//----------------- Shortest Job First (Preemptive) End ------------------------

//-------------------------- LRTF Algo Start -----------------------------
void LRTF(vector<Process> p) {
    int n = p.size(), t = 0, completed = 0;
    int prev_idx = -1; // -1: no process, valid_idx: process Px, -2: idle
    for (auto &pr : p)
        pr.remaining_bt = pr.bt;
    vector<tuple<int, int, string>> timeline_gantt;

    int current_block_start_time = 0;

    while (completed < n) {
        int idx = -1;
        int max_rem_bt = -1; // Longest Remaining Burst Time
        int min_at_tie_breaker = INT_MAX; // Earliest Arrival Time (for tie-breaking)


        for (int i = 0; i < n; i++) {
            if (p[i].at <= t && p[i].remaining_bt > 0) {
                // Tie-breaking: larger Remaining BT, then smaller AT, then smaller PID
                if (p[i].remaining_bt > max_rem_bt) {
                    max_rem_bt = p[i].remaining_bt;
                    min_at_tie_breaker = p[i].at;
                    idx = i;
                } else if (p[i].remaining_bt == max_rem_bt) {
                    if (p[i].at < min_at_tie_breaker) {
                        min_at_tie_breaker = p[i].at;
                        idx = i;
                    } else if (p[i].at == min_at_tie_breaker && p[i].pid < p[idx].pid) {
                        idx = i;
                    }
                }
            }
        }

        if (idx == -1) { // CPU is Idle
            if (prev_idx != -2) { // If previous state was not idle, close the previous block
                if (prev_idx != -1) {
                    timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
                }
                current_block_start_time = t; // Start of new idle block
            }
            if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) == t) {
                get<1>(timeline_gantt.back())++;
            } else {
                 timeline_gantt.emplace_back(t, t+1, "Idle");
            }
            prev_idx = -2; // Mark as idle state
            t++;
            continue;
        }

        // A process is selected (idx is valid)
        if (prev_idx != idx) { // Context switch occurred (or coming from idle)
            if (prev_idx != -1) { // If there was a previous block
                if (prev_idx == -2) {
                     if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<0>(timeline_gantt.back()) == current_block_start_time) {
                         get<1>(timeline_gantt.back()) = t;
                     }
                } else {
                    timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
                }
            }
            current_block_start_time = t; // Start of new process block
            prev_idx = idx; // Update previous process
        }

        if (p[idx].remaining_bt == p[idx].bt) {
            p[idx].start_time = t; // Record first start time
        }

        p[idx].remaining_bt--;
        t++;

        if (p[idx].remaining_bt == 0) {
            // Process finished
            p[idx].ct = t;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            p[idx].rt = p[idx].start_time - p[idx].at;
            completed++;

            timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[idx].pid));
            prev_idx = -1; // Reset prev_idx
            current_block_start_time = t;
        }
    }
    if (prev_idx != -1 && prev_idx != -2 && current_block_start_time < t) {
        timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
    } else if (prev_idx == -2 && current_block_start_time < t) {
        if(!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) < t) {
            get<1>(timeline_gantt.back()) = t;
        }
    }
    printGanttChart(timeline_gantt);
    printTable(p);
}
//-------------------------- LRTF Algo End --------------------------


//-------------------------- Round Robin Algo Start ----------------------------
void RoundRobin(vector<Process> p_original, int tq) {
    int n = p_original.size();
    if (n == 0) {
        printStyled("\nNo processes to schedule.", "yellow", false);
        vector<tuple<int, int, string>> empty_timeline;
        printGanttChart(empty_timeline);
        printTable(p_original);
        return;
    }

    vector<tuple<int, int, string>> timeline_gantt;
    vector<Process> p = p_original; // Working copy
    sort(p.begin(), p.end(), sortByAT);

    vector<int> current_remaining_bt(n);
    vector<int> first_response_time(n, -1); // For RT calculation
    vector<int> completion_time_val(n, 0);  // For CT calculation

    for (int i = 0; i < n; ++i) {
        current_remaining_bt[i] = p[i].bt;
        p[i].start_time = -1; // Reset for this simulation run
    }

    queue<int> ready_queue;
    vector<bool> in_queue(n, false);

    int current_time = 0;
    int completed_processes = 0;

    // Gantt chart specific state
    int current_gantt_entity_pid = -1; // -1: None, 0: Idle, >0: Process PID
    int current_gantt_block_start_time = 0;

    // Initial population of ready queue
    for (int i = 0; i < n; ++i) {
        if (p[i].at <= current_time && current_remaining_bt[i] > 0 && !in_queue[i]) {
            ready_queue.push(i);
            in_queue[i] = true;
        }
    }

    while (completed_processes < n) {
        // Populate ready queue with newly arrived processes
        for (int i = 0; i < n; ++i) {
            if (p[i].at <= current_time && current_remaining_bt[i] > 0 && !in_queue[i]) {
                ready_queue.push(i);
                in_queue[i] = true;
            }
        }

        if (ready_queue.empty()) {
            // --- CPU is Idle ---
            int next_arrival_time = INT_MAX;
            bool found_pending_process = false;
            for (int i = 0; i < n; ++i) {
                if (current_remaining_bt[i] > 0) { // If process is not yet completed
                    found_pending_process = true;
                    if (p[i].at > current_time) { // Arrives in the future
                        next_arrival_time = min(next_arrival_time, p[i].at);
                    } else { // Arrived or is arriving now but wasn't in queue (should be caught by populate)
                        // This case implies a process is ready now.
                        next_arrival_time = min(next_arrival_time, current_time);
                    }
                }
            }

            if (!found_pending_process) break; // All processes completed

            if (next_arrival_time == INT_MAX) { // No future arrivals, but some processes might be pending (error or completed)
                 break;
            }
            
            if (current_time < next_arrival_time) { // Only log idle if there's a time gap
                if (current_gantt_entity_pid != 0) { // If not already idle
                    if (current_gantt_entity_pid > 0) { // If a process was running
                        timeline_gantt.emplace_back(current_gantt_block_start_time, current_time, "P" + to_string(current_gantt_entity_pid));
                    }
                    current_gantt_entity_pid = 0; // Mark as Idle
                    current_gantt_block_start_time = current_time;
                }
            }
            // If an idle block is active and continues, its end time will be updated if next_arrival_time extends further
            // For distinct idle blocks:
            if (current_time < next_arrival_time) {
                // If the *last* block added was this idle block, we'd extend it.
                // However, simpler to just add a new block if the previous wasn't idle ending now.
                // The check `current_gantt_entity_pid != 0` handles transition TO idle.
                // If already idle, `current_gantt_entity_pid` is 0.
            }
            
            current_time = next_arrival_time; // Advance time
            // The actual idle block will be added when Idle transitions to something else,
            // or if we decide to add distinct idle blocks here using (old_current_time, new_current_time, "Idle")
            // For simplicity with current_gantt_entity_pid, let's ensure Idle blocks are closed when transitioning away from Idle.
            // If current_time was X and advanced to Y, and CPU was idle:
            if (current_gantt_entity_pid == 0 && current_gantt_block_start_time < current_time) {
                 // This implies an idle period from current_gantt_block_start_time to current_time (new)
                 // This is tricky because current_time has already advanced.
                 // Let's refine: if CPU becomes idle, record previous block, mark state as idle.
                 // The idle block itself is recorded when it *ends*.
            }
            continue; // Re-check queue at new current_time
        }

        // --- Process Execution ---
        int proc_idx = ready_queue.front();
        ready_queue.pop();
        in_queue[proc_idx] = false;

        Process& current_process = p[proc_idx];

        if (current_gantt_entity_pid != current_process.pid) { // Switched to a new process or from Idle/None
            if (current_gantt_entity_pid > 0) { // If a different process was running
                timeline_gantt.emplace_back(current_gantt_block_start_time, current_time, "P" + to_string(current_gantt_entity_pid));
            } else if (current_gantt_entity_pid == 0) { // If it was Idle
                timeline_gantt.emplace_back(current_gantt_block_start_time, current_time, "Idle");
            }
            current_gantt_entity_pid = current_process.pid;
            current_gantt_block_start_time = current_time;
        }

        if (first_response_time[proc_idx] == -1) {
            first_response_time[proc_idx] = current_time;
        }

        int time_slice = min(tq, current_remaining_bt[proc_idx]);
        current_time += time_slice;
        current_remaining_bt[proc_idx] -= time_slice;

        // Add newly arrived processes during this time_slice execution
        for (int i = 0; i < n; ++i) {
            if (p[i].at <= current_time && p[i].at > (current_time - time_slice) && // Arrived during this slice
                current_remaining_bt[i] > 0 && !in_queue[i] && i != proc_idx) {
                ready_queue.push(i);
                in_queue[i] = true;
            }
        }

        if (current_remaining_bt[proc_idx] > 0) {
            ready_queue.push(proc_idx); // Add back to queue
            in_queue[proc_idx] = true;
        } else {
            // Process completed
            completion_time_val[proc_idx] = current_time;
            completed_processes++;
            // Close its Gantt block
            timeline_gantt.emplace_back(current_gantt_block_start_time, current_time, "P" + to_string(current_process.pid));
            current_gantt_entity_pid = -1; // Mark CPU as free momentarily
            current_gantt_block_start_time = current_time; // Next block starts now
        }
    }

    // If the last running entity was a process that didn't complete (loop ended by time or other condition)
    // or if the last state was idle and it needs to be closed.
    if (current_gantt_entity_pid > 0) { // A process was last running
        timeline_gantt.emplace_back(current_gantt_block_start_time, current_time, "P" + to_string(current_gantt_entity_pid));
    } else if (current_gantt_entity_pid == 0 && current_gantt_block_start_time < current_time) { // Ended in an Idle state
        timeline_gantt.emplace_back(current_gantt_block_start_time, current_time, "Idle");
    }

    // Update process metrics in 'p' (which is sorted by AT, then PID)
    for (int i = 0; i < n; ++i) {
        p[i].ct = completion_time_val[i];
        if (first_response_time[i] != -1) { // If process ran
            p[i].tat = p[i].ct - p[i].at;
            p[i].wt = p[i].tat - p[i].bt;
            p[i].rt = first_response_time[i] - p[i].at;
        } else { // Process never ran
            p[i].ct = p[i].at; // Or some other default
            p[i].tat = 0;
            p[i].wt = 0;
            p[i].rt = 0; // Or -1
        }
        // Ensure non-negative WT and RT (can happen if CT/start_time recorded slightly before AT due to time=0 handling)
        if(p[i].wt < 0) p[i].wt = 0;
        if(p[i].rt < 0) p[i].rt = 0;
    }

    // Sort 'p' by PID for printing, as it currently holds all metrics
    sort(p.begin(), p.end(), [](const Process &a, const Process &b) {
        return a.pid < b.pid;
    });

    printGanttChart(timeline_gantt);
    printTable(p);
}

// ----------------------- Round Robin Algo End -----------------------------------

//------------------ Priority Scheduling (Non-Preemptive) Start ----------------------
void Priority_NonPreemptive(vector<Process> p) {
    int n = p.size(), t = 0, completed = 0;
    vector<tuple<int, int, string>> timeline_gantt;
    for (auto &pr : p)
        pr.visited = false;

    int current_block_start_time = 0;

    while (completed < n) {
        int idx = -1;
        int highest_priority = INT_MAX; // Smaller number = higher priority
        int min_at_tie_breaker = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (!p[i].visited && p[i].at <= t) {
                // Tie-breaking: smaller Priority (higher priority), then smaller AT, then smaller PID
                if (p[i].priority < highest_priority) {
                    highest_priority = p[i].priority;
                    min_at_tie_breaker = p[i].at;
                    idx = i;
                } else if (p[i].priority == highest_priority) {
                    if (p[i].at < min_at_tie_breaker) {
                        min_at_tie_breaker = p[i].at;
                        idx = i;
                    } else if (p[i].at == min_at_tie_breaker && p[i].pid < p[idx].pid) {
                        idx = i;
                    }
                }
            }
        }
        if (idx == -1) { // CPU is Idle
            if (timeline_gantt.empty() || get<2>(timeline_gantt.back()) != "Idle" || get<1>(timeline_gantt.back()) != t) {
                if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) != "Idle" && get<1>(timeline_gantt.back()) == t) {
                    // This means a process just finished at time `t`, and now CPU is idle from `t`
                    // So we must start a new idle block from `t`.
                }
                timeline_gantt.emplace_back(t, t + 1, "Idle");
            } else {
                get<1>(timeline_gantt.back())++; // Extend last idle block
            }
            t++;
            current_block_start_time = t;
        } else {
            // Process selected. If there was an idle block, close it.
            if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) > t) {
                get<1>(timeline_gantt.back()) = t; // Trim the idle block
            } else if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<0>(timeline_gantt.back()) == t) {
                 timeline_gantt.pop_back(); // Remove 0-duration idle block
            }

            p[idx].start_time = t;
            p[idx].ct = t + p[idx].bt;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].start_time - p[idx].at;
            p[idx].rt = p[idx].wt; // For non-preemptive, response time = waiting time
            p[idx].visited = true;

            timeline_gantt.emplace_back(t, p[idx].ct, "P" + to_string(p[idx].pid));
            t = p[idx].ct;
            completed++;
            current_block_start_time = t;
        }
    }
    printGanttChart(timeline_gantt);
    printTable(p, true); // Pass true to show priority column
}
//-------------------------  Priority Scheduling (Non-Preemptive) End ---------------------------------- 


//------------------------- Priority Scheduling (Preemptive) Start -----------------------------------
void Priority_Preemptive(vector<Process> p) {
    int n = p.size(), t = 0, completed = 0;
    int prev_idx = -1; // -1: no process, valid_idx: process Px, -2: idle
    for (auto &pr : p)
        pr.remaining_bt = pr.bt;
    vector<tuple<int, int, string>> timeline_gantt;

    int current_block_start_time = 0;

    while (completed < n) {
        int idx = -1;
        int highest_priority = INT_MAX;
        int min_at_tie_breaker = INT_MAX;


        for (int i = 0; i < n; i++) {
            if (p[i].at <= t && p[i].remaining_bt > 0) {
                // Tie-breaking: smaller Priority (higher priority), then smaller AT, then smaller PID
                if (p[i].priority < highest_priority) {
                    highest_priority = p[i].priority;
                    min_at_tie_breaker = p[i].at;
                    idx = i;
                } else if (p[i].priority == highest_priority) {
                    if (p[i].at < min_at_tie_breaker) {
                        min_at_tie_breaker = p[i].at;
                        idx = i;
                    } else if (p[i].at == min_at_tie_breaker && p[i].pid < p[idx].pid) {
                        idx = i;
                    }
                }
            }
        }

        if (idx == -1) { // CPU is Idle
            if (prev_idx != -2) { // If previous state was not idle, close the previous block
                if (prev_idx != -1) {
                    timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
                }
                current_block_start_time = t; // Start of new idle block
            }
            if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) == t) {
                get<1>(timeline_gantt.back())++;
            } else {
                 timeline_gantt.emplace_back(t, t+1, "Idle");
            }
            prev_idx = -2; // Mark as idle state
            t++;
            continue;
        }

        // A process is selected (idx is valid)
        if (prev_idx != idx) { // Context switch occurred (or coming from idle)
            if (prev_idx != -1) { // If there was a previous block
                if (prev_idx == -2) {
                     if (!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<0>(timeline_gantt.back()) == current_block_start_time) {
                         get<1>(timeline_gantt.back()) = t;
                     }
                } else {
                    timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
                }
            }
            current_block_start_time = t; // Start of new process block
            prev_idx = idx; // Update previous process
        }

        if (p[idx].remaining_bt == p[idx].bt) {
            p[idx].start_time = t; // Record first start time
        }

        p[idx].remaining_bt--;
        t++;

        if (p[idx].remaining_bt == 0) {
            // Process finished
            p[idx].ct = t;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            p[idx].rt = p[idx].start_time - p[idx].at;
            completed++;

            timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[idx].pid));
            prev_idx = -1; // Reset prev_idx
            current_block_start_time = t;
        }
    }
    if (prev_idx != -1 && prev_idx != -2 && current_block_start_time < t) {
        timeline_gantt.emplace_back(current_block_start_time, t, "P" + to_string(p[prev_idx].pid));
    } else if (prev_idx == -2 && current_block_start_time < t) {
        if(!timeline_gantt.empty() && get<2>(timeline_gantt.back()) == "Idle" && get<1>(timeline_gantt.back()) < t) {
            get<1>(timeline_gantt.back()) = t;
        }
    }
    printGanttChart(timeline_gantt);
    printTable(p, true); // Pass true to show priority column
}
// ------------------------ Priority Scheduling (Preemptive) End -------------------------------

void run() {
    CLEAR_SCREEN();
    printWelcome();

    while (true) {
        int choice;
        cout << "\n\n";
        printStyled(" CPU Scheduling Algorithms Menu ", "green", true);
        cout << "\n1. First Come First Serve (FCFS)"
             << "\n2. Shortest Job First (Non-Preemptive)"
             << "\n3. Shortest Job First (Preemptive)"
             << "\n4. Longest Remaining Time First (LRTF)"
             << "\n5. Round Robin"
             << "\n6. Priority Scheduling (Non-Preemptive)"
             << "\n7. Priority Scheduling (Preemptive)"
             << "\n8. Exit"
             << "\n\nEnter your choice (1-8): ";
        cin >> choice;

        // Clear input buffer after reading choice
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        CLEAR_SCREEN();

        vector<Process> processes;
        int time_quantum = 0;

        // Get process input based on the chosen algorithm
        if (choice >= 1 && choice <= 7) {
            if (choice == 5) { // Round Robin needs time quantum
                processes = getProcessInput();
                cout << "Enter Time Quantum for Round Robin: ";
                cin >> time_quantum;
                // Clear input buffer again after reading time_quantum
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            } else if (choice == 6 || choice == 7) { // Priority algorithms need priority
                processes = getProcessInput(true);
            } else if (choice >= 1 && choice <= 4) { // Other algorithms only need AT and BT
                processes = getProcessInput();
            }
        }

        switch (choice) {
            case 1:
                printStyled("--- First Come First Serve (FCFS) ---", "cyan", true);
                FCFS(processes);
                break;
            case 2:
                printStyled("--- Shortest Job First (Non-Preemptive) ---", "cyan", true);
                SJF_NonPreemptive(processes);
                break;
            case 3:
                printStyled("--- Shortest Job First (Preemptive) ---", "cyan", true);
                SJF_Preemptive(processes);
                break;
            case 4:
                printStyled("--- Longest Remaining Time First (LRTF) ---", "cyan", true);
                LRTF(processes);
                break;
            case 5:
                printStyled("--- Round Robin Scheduling ---", "cyan", true);
                RoundRobin(processes, time_quantum);
                break;
            case 6:
                printStyled("--- Priority Scheduling (Non-Preemptive) ---", "cyan", true);
                Priority_NonPreemptive(processes);
                break;
            case 7:
                printStyled("--- Priority Scheduling (Preemptive) ---", "cyan", true);
                Priority_Preemptive(processes);
                break;
            case 8:
                printStyled("\nExiting... Thank you!", "yellow", true);
                return;
            default:
                printStyled("\nInvalid choice! Please try again.", "red", true);
        }

        cout << "\n\nPress any key to return to the menu...";
        GET_CHAR(); // Using the cross-platform GET_CHAR macro
        CLEAR_SCREEN();
    }

}

void launch_scheduler(){
    run();
}