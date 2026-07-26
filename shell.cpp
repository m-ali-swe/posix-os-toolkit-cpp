// Simple UNIX-style Shell Implementation in C++
// This file contains an interactive shell with support for pipes, I/O redirection,
// built-in commands (cd, pwd, history, echo, exit), and basic signal handling.

#include <iostream>     // For std::cout, std::cerr
#include <string>       // For std::string
#include <vector>       // For std::vector
#include <sstream>      // For std::stringstream
#include <cstring>      // For std::strerror
#include <unistd.h>     // For fork, execvp, getcwd, chdir, pipe, dup2
#include <sys/wait.h>   // For waitpid, WIFEXITED, WEXITSTATUS
#include <fcntl.h>      // For open flags
#include <limits.h>     // For PATH_MAX
#include <cstdlib>      // For getenv, exit, setenv, unsetenv
#include <csignal>      // For signal handling (sig_atomic_t, signal)
#include "shell.h"

using namespace std;

// ==================== Global Variables ====================
// history_list: stores each command line entered by the user
vector<string> history_list;

// previous_dir: holds the previous working directory (for "cd -" builtin)
string previous_dir = "";

// sigint_received: flag set by SIGINT handler to break out of shell loop
volatile sig_atomic_t sigint_received = 0;

// ANSI color codes for prompt styling
const string COLOR_RESET = "\033[0m";
const string COLOR_RED = "\033[1;31m"; // Red-red for prompt
const string COLOR_BLUE  = "\033[1;34m";

// ==================== Command Structure ====================
// Holds parsed data for each simple command (one segment of a pipeline):
// - args: vector of tokens (program + arguments)
// - input_file: path if '<' redirection is used
// - output_file: path if '>' or '>>' is used
// - append_output: true if '>>', false if '>'
struct Command {
    vector<string> args;
    string input_file;
    string output_file;
    bool append_output = false;

    // get_argv(): converts args to a NULL-terminated C array for execvp()
    char** get_argv() {
        if (args.empty()) return nullptr;
        char** argv = new char*[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = const_cast<char*>(args[i].c_str());
        }
        argv[args.size()] = nullptr;  // Must end with NULL
        return argv;
    }

    // free_argv(): deletes the allocated array (not the C-strings)
    void free_argv(char** argv) {
        delete[] argv;
    }
};

// ==================== Function Prototypes ====================
void sigint_handler(int sig);
void display_welcome_banner();
string get_full_current_dir();
string get_current_dir();
void parse_line(const string& line, vector<Command>& commands, bool& background);
bool is_builtin(const string& name);
void handle_builtin(const Command& cmd, int& status);
int execute_commands(vector<Command> commands, bool background);
void run_shell();

// ==================== SIGINT Handler ====================
// Catches Ctrl+C (SIGINT) and sets a flag so the shell can exit gracefully.
void sigint_handler(int /*sig*/) {
    sigint_received = 1;
}

// ==================== Utility: Full CWD ====================
// Returns the absolute current working directory, or "?" on error.
string get_full_current_dir() {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == nullptr) {
        perror("getcwd failed");
        return string("?");
    }
    return string(buf);
}

// ==================== Utility: Prompt CWD ====================
// Returns '~'-prefixed path if under $HOME, else full path.
string get_current_dir() {
    string cwd = get_full_current_dir();
    const char* home = getenv("HOME");
    if (home) {
        string homes(home);
        // If cwd == home or starts with home + '/', replace prefix
        if (cwd == homes) return "~";
        if (cwd.rfind(homes + "/", 0) == 0) {
            return "~" + cwd.substr(homes.size());
        }
    }
    return cwd;
}

// ==================== display_welcome_banner ====================
// Prints a static ASCII art banner and version info.
void display_welcome_banner() {
    cout << COLOR_RED << R"(
Welcome to...
  __  __  __    __   _____ _          _ _
 |  \/  | \ \  / /  / ____| |        | | |
 | \  / |  \ \/ /  | (___ | |__   ___| | |
 | |\/| |   \  /    \___ \| '_ \ / _ \ | |
 | |  | |   / /     ____) | | | |  __/ | |
 |_|  |_|  /_/     |_____/|_| |_|\___|_|_|
                                  v0.2
)" << COLOR_RESET << endl;
    cout << COLOR_BLUE << "----------------------" << COLOR_RESET << endl << endl;
}

// ==================== parse_line ====================
// Splits the raw input line on '|' into segments, then tokenizes each
// segment on whitespace to fill Command.args, handling '<', '>', '>>'.
// Sets 'background' true if '&' is trailing (removed before parsing).
void parse_line(const string& line, vector<Command>& commands, bool& background) {
    commands.clear();
    background = false;
    string s = line;
    
    // Check for '&' at end
    if (!s.empty() && s.back() == '&') {
        background = true;
        s.pop_back();
    }
    
    stringstream ss(s);
    string segment;
    while (getline(ss, segment, '|')) {
        Command cmd;
        stringstream segseg(segment);
        string token;
        while (segseg >> token) {
            if (token == ">") {
                segseg >> cmd.output_file;
                cmd.append_output = false;
            } else if (token == ">>") {
                segseg >> cmd.output_file;
                cmd.append_output = true;
            } else if (token == "<") {
                segseg >> cmd.input_file;
            } else {
                cmd.args.push_back(token);
            }
        }
        if (!cmd.args.empty()) commands.push_back(cmd);
    }
}

// ==================== is_builtin ====================
// Returns true if name matches one of our built-in commands.
bool is_builtin(const string& name) {
    return name == "exit" || name == "cd" || name == "pwd" ||
           name == "history" || name == "echo";
}

// ==================== handle_builtin ====================
// Executes built-in logic directly in the shell process.
void handle_builtin(const Command& cmd, int& status) {
    status = 0;
    const string& name = cmd.args[0];
    if (name == "exit") {
        exit(0);
    }
    if (name == "cd") {
        string target = (cmd.args.size() < 2 || cmd.args[1] == "~")
                        ? getenv("HOME") : cmd.args[1];
        if (target == "-") target = previous_dir;
        string old_dir=get_full_current_dir();
        if (chdir(target.c_str()) < 0) {
            perror("cd failed"); status = 1;
        } else {
            previous_dir = old_dir;
        }
        return;
    }
    if (name == "pwd") {
        cout << get_full_current_dir() << endl;
        return;
    }
    if (name == "history") {
        for (size_t i = 0; i < history_list.size(); ++i) {
            cout << i+1 << "  " << history_list[i] << endl;
        }
        return;
    }
    if (name == "echo") {
        for (size_t i=1; i<cmd.args.size(); ++i) {
            cout << cmd.args[i] << (i+1<cmd.args.size()?" ":"");
        }
        cout << endl;
        return;
    }
}

// ==================== execute_commands ====================
// Runs one or more Commands as a pipeline, handling I/O redirection.
int execute_commands(vector<Command> cmds, bool background) {
    int status = 0;
    if (cmds.empty()) return 0;

    // Single built-in with no background: handle inline
    if (cmds.size()==1 && is_builtin(cmds[0].args[0]) && !background) {
        handle_builtin(cmds[0], status);
        return status;
    }

    int in_fd = STDIN_FILENO;
    pid_t last_pid = -1;

    for (size_t i = 0; i < cmds.size(); ++i) {
        int pipe_fd[2] = {-1,-1};
        bool do_pipe = (i+1 < cmds.size());
        if (do_pipe && pipe(pipe_fd) < 0) { perror("pipe"); return 1; }

        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) {
            // Child
            if (in_fd != STDIN_FILENO) { dup2(in_fd, STDIN_FILENO); close(in_fd); }
            if (do_pipe) { dup2(pipe_fd[1], STDOUT_FILENO); close(pipe_fd[0]); close(pipe_fd[1]); }
            if (!cmds[i].input_file.empty()) {
                int fd = open(cmds[i].input_file.c_str(), O_RDONLY);
                dup2(fd, STDIN_FILENO); close(fd);
            }
            if (!cmds[i].output_file.empty()) {
                int flags = O_WRONLY|O_CREAT|(cmds[i].append_output?O_APPEND:O_TRUNC);
                int fd = open(cmds[i].output_file.c_str(), flags, 0644);
                dup2(fd, STDOUT_FILENO); close(fd);
            }
            if (is_builtin(cmds[i].args[0])) {
                int st=0; handle_builtin(cmds[i], st); exit(st);
            }
            char** argv = cmds[i].get_argv();
            execvp(argv[0], argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        // Parent
        last_pid = pid;
        if (in_fd != STDIN_FILENO) close(in_fd);
        if (do_pipe) { close(pipe_fd[1]); in_fd = pipe_fd[0]; }
    }

    if (!background && last_pid > 0) {
        waitpid(last_pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
    return 0;
}

// ==================== run_shell (main loop) ====================
// Prints banner, then loops: prompt -> read line -> parse -> execute
void run_shell() {
    sigint_received = 0;
    signal(SIGINT, sigint_handler);
    previous_dir = get_full_current_dir();
    display_welcome_banner();

    string line;
    while (!sigint_received) {
        cout << COLOR_RED << "myShell@System" << COLOR_RESET << ":"
             << COLOR_BLUE << get_current_dir() << COLOR_RESET << "$ ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        history_list.push_back(line);
        vector<Command> cmds;
        bool bg = false;
        parse_line(line, cmds, bg);
        if (!cmds.empty() && cmds[0].args[0] == "exit") break;
        execute_commands(cmds, bg);
    }
    system("clear");
    cout <<COLOR_RED<< "\nExiting shell." << endl;
}
