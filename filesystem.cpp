#include <iostream>
#include <string>
#include <vector>
#include <map>        // For std::map in VFS
#include <fstream>    // For file saving/loading in VFS
#include <sstream>    // For string stream parsing in VFS
#include <cstdlib>    // For system("cls") / system("clear") - basic clear screen
#include <limits>     // For std::numeric_limits (used in input clearing)
#include <algorithm>  // For std::sort
#include <cstddef>    // For size_t
using namespace std;

// For platform-dependent sleep
#ifdef _WIN32
#include <windows.h> // For Sleep
#else
#include <unistd.h>  // For usleep
#endif

//-----------------------------------------------------------------------------
// ANSI Escape Code Graphics Enhancements
//-----------------------------------------------------------------------------
namespace ConsoleGFX {

    // ANSI Color Codes
    const std::string RESET = "\033[0m";
    const std::string BLACK = "\033[30m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    // Bold versions
    const std::string BOLD_BLACK = "\033[1;30m";
    const std::string BOLD_RED = "\033[1;31m";
    const std::string BOLD_GREEN = "\033[1;32m";
    const std::string BOLD_YELLOW = "\033[1;33m";
    const std::string BOLD_BLUE = "\033[1;34m";
    const std::string BOLD_MAGENTA = "\033[1;35m";
    const std::string BOLD_CYAN = "\033[1;36m";
    const std::string BOLD_WHITE = "\033[1;37m";

    // Simple clear screen (basic platform check)
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            // Assume POSIX/ANSI compatible terminal
             // "\033[2J" clears the screen
             // "\033[H" moves the cursor to the home position (top-left)
            std::cout << "\033[2J\033[H" << std::flush;
        #endif
    }

    // Basic Loading Animation (Spinner)
    // Uses carriage return '\r' to overwrite the spinner character in place.
    void showLoading(const std::string& message = "Processing", int duration_ms = 1000) {
        std::cout << YELLOW << message << " " << RESET << std::flush;
        const char spinner[] = {'|', '/', '-', '\\'};
        const int delay_ms = 100; // Update frequency
        int num_updates = duration_ms / delay_ms;
        int spinner_index = 0;

        for (int i = 0; i < num_updates; ++i) {
            // Print the spinner character, move cursor back, flush output
            std::cout << BOLD_YELLOW << spinner[spinner_index] << "\r" << RESET << std::flush;
            spinner_index = (spinner_index + 1) % 4; // Cycle through spinner characters
            // Platform-dependent sleep
            #ifdef _WIN32
                Sleep(delay_ms);
            #else
                usleep(static_cast<useconds_t>(delay_ms * 1000)); // usleep takes microseconds
            #endif
        }
        // Clear the spinner area by printing spaces and moving cursor back
        std::cout << "  \r" << std::flush;
    }

} // namespace ConsoleGFX

//-----------------------------------------------------------------------------
// Virtual File System (VFS) Implementation
//-----------------------------------------------------------------------------

class File {
public:
    std::string name;
    std::string content;

    File(std::string name, std::string content = "") : name(name), content(content) {}
     // Destructor if needed (optional for simple strings)
    ~File() = default;
};

class Directory {
public:
    std::string name;
    std::map<std::string, Directory*> subdirs;
    std::map<std::string, File*> files;
    Directory* parent;

    Directory(std::string name, Directory* parent = nullptr) : name(name), parent(parent) {}

    // Destructor to clean up children
    ~Directory() {
        for (auto& sub : subdirs)
            delete sub.second;
        for (auto& file : files)
            delete file.second;
    }
};

class VirtualFileSystem {
private:
    Directory* root;
    Directory* current;

    // Helper for destructor and deleteDirectoryByName
    void deleteDirectoryRecursive(Directory* dir) {
         if (!dir) return; // Added null check
        for (auto& sub : dir->subdirs)
            deleteDirectoryRecursive(sub.second);
        for (auto& file : dir->files)
            delete file.second;
         dir->subdirs.clear(); // Clear map after deleting pointers
         dir->files.clear();   // Clear map after deleting pointers
        delete dir;
    }

    void saveDirectory(std::ofstream& out, Directory* dir, const std::string& path) {
        if (!dir) return; // Added null check
        // Only add dir->name to path if it's not the root directory being processed recursively
        std::string current_path_segment = (dir == root && path.empty()) ? "" : dir->name;
        std::string dirPath = path.empty() ? current_path_segment : path + "/" + current_path_segment;

         // Don't write "DIR /" for the absolute root path, only for subdirectories
         if (!dirPath.empty()) {
             out << "DIR " << dirPath << std::endl;
         }


        for (auto& file : dir->files) {
            std::string filePath = dirPath.empty() ? file.second->name : dirPath + "/" + file.second->name;
            out << "FILE " << filePath << " " << file.second->content << std::endl;
        }
        for (auto& sub : dir->subdirs)
            saveDirectory(out, sub.second, dirPath);
    }


    // Creates a directory path if it doesn't exist, relative to root
    Directory* createPathFromRoot(const std::string& path) {
        std::stringstream ss(path);
        std::string part;
        Directory* temp = root;
        Directory* prev = nullptr;
        (void)prev;

         // Skip initial '/' if present
         if (path.size() > 0 && path[0] == '/') {
             ss.ignore(1);
         }

        while (std::getline(ss, part, '/')) {
            if (part.empty() || part == ".") continue; // Ignore empty parts and current dir
            if (part == "..") {
                 if (temp->parent) {
                    temp = temp->parent;
                 }
            } else {
                if (!temp->subdirs.count(part)) {
                    temp->subdirs[part] = new Directory(part, temp);
                }
                prev = temp; // Keep track of the parent before moving
                temp = temp->subdirs[part];
            }
        }
         // If the path ends with a directory name that didn't exist, temp is the new directory
        return temp;
    }


    void loadFromDisk() {
        std::ifstream in("filesystem.txt");
        if (!in.is_open()) {
             std::cerr << ConsoleGFX::YELLOW << "filesystem.txt not found. Starting with empty VFS." << ConsoleGFX::RESET << std::endl;
             return; // File doesn't exist, start fresh
        }
        std::string line;
        while (std::getline(in, line)) {
            std::stringstream ss(line);
            std::string type, fullPath, content;
            ss >> type;
            // Read the full path, which might contain spaces
            std::getline(ss, fullPath, ' '); // Read until the first space after type

            // Trim leading/trailing whitespace from fullPath
            fullPath.erase(0, fullPath.find_first_not_of(" \t\n\r\f\v"));
            fullPath.erase(fullPath.find_last_not_of(" \t\n\r\f\v") + 1);


            if (type == "DIR") {
                if (!fullPath.empty()) { // Don't try to create root/root or just root
                   createPathFromRoot(fullPath);
                }
            } else if (type == "FILE") {
                // Read the rest of the line as content
                std::getline(ss, content);

                size_t pos = fullPath.find_last_of('/');
                std::string dirPath = "";
                std::string fileName;

                if (pos == std::string::npos) {
                    // File in root directory (path is just filename)
                    fileName = fullPath;
                    dirPath = ""; // Root's path is considered empty for createPathFromRoot
                } else {
                    dirPath = fullPath.substr(0, pos);
                    fileName = fullPath.substr(pos + 1);
                }

                Directory* dir = createPathFromRoot(dirPath);

                if (dir) {
                    // Ensure file doesn't exist before creating
                    if (!dir->files.count(fileName)) {
                         // content.substr(1) to remove the leading space read by getline
                        dir->files[fileName] = new File(fileName, content.substr(1));
                    } else {
                         // Handle case where file already exists during load (e.g., duplicate lines)
                         dir->files[fileName]->content = content.substr(1); // Overwrite content
                    }
                } else {
                     std::cerr << ConsoleGFX::RED << "Error loading file: Directory for '" << fullPath << "' not found during load." << ConsoleGFX::RESET << std::endl;
                }
            }
        }
        in.close();
    }

    // Recursive helper to print the directory tree
    void printDirectoryTreeRecursive(Directory* dir, const std::string& prefix, bool is_last_sibling) const {
        if (!dir) return;

        // Determine the connector for the current item line
        std::string connector = is_last_sibling ? "`-- " : "|-- ";
        // Determine the prefix for the children's lines
        std::string child_prefix = prefix + (is_last_sibling ? "    " : "|   ");

        // Print current directory name (skip for the initial root call)
        if (dir != root) {
            std::cout << prefix << connector
                      << ConsoleGFX::BOLD_BLUE << dir->name << "/" << ConsoleGFX::RESET << std::endl;
        }

        // Sort subdirectories and files for consistent output
        std::vector<std::pair<std::string, Directory*>> sorted_subdirs;
        for(const auto& pair : dir->subdirs) sorted_subdirs.push_back(pair);
        std::sort(sorted_subdirs.begin(), sorted_subdirs.end(), [](const auto& a, const auto& b){ return a.first < b.first; });

        std::vector<std::pair<std::string, File*>> sorted_files;
        for(const auto& pair : dir->files) sorted_files.push_back(pair);
        std::sort(sorted_files.begin(), sorted_files.end(), [](const auto& a, const auto& b){ return a.first < b.first; });

        size_t total_items = sorted_subdirs.size() + sorted_files.size();
        size_t current_item_index = 0;

        // Print subdirectories
        for (const auto& pair : sorted_subdirs) {
            current_item_index++;
            bool is_last_item = (current_item_index == total_items);
            printDirectoryTreeRecursive(pair.second, child_prefix, is_last_item);
        }

        // Print files
        for (const auto& pair : sorted_files) {
            current_item_index++;
            bool is_last_item = (current_item_index == total_items);
            std::cout << child_prefix << (is_last_item ? "`-- " : "|-- ")
                      << ConsoleGFX::GREEN << pair.first << ConsoleGFX::RESET << std::endl;
        }
    }


public:
    VirtualFileSystem() {
        root = new Directory("root");
        current = root;
        loadFromDisk();
    }

    ~VirtualFileSystem() {
        std::ofstream out("filesystem.txt");
        if (out.is_open()) {
            // Save the contents of the root directory
            saveDirectory(out, root, "");
            out.close();
        } else {
             std::cerr << ConsoleGFX::RED << "Error: Could not save filesystem.txt" << ConsoleGFX::RESET << std::endl;
        }
        // Delete contents of root, then root itself
        // The Directory destructor handles recursive deletion of children
        delete root; // This calls root's destructor which cleans up
    }

    void createDirectory(const std::string& name) {
        if (name.empty() || name.find('/') != std::string::npos || name == "." || name == "..") {
            std::cout << ConsoleGFX::RED << "Invalid directory name." << ConsoleGFX::RESET << std::endl;
            return;
        }
        ConsoleGFX::showLoading("Creating directory", 500);
        if (current->subdirs.count(name)) {
            std::cout << ConsoleGFX::YELLOW << "Directory '" << name << "' already exists." << ConsoleGFX::RESET << std::endl;
            return;
        }
        current->subdirs[name] = new Directory(name, current);
        std::cout << ConsoleGFX::GREEN << "Directory '" << name << "' created." << ConsoleGFX::RESET << std::endl;
    }

    void createFile(const std::string& name) {
         if (name.empty() || name.find('/') != std::string::npos || name == "." || name == "..") {
            std::cout << ConsoleGFX::RED << "Invalid file name." << ConsoleGFX::RESET << std::endl;
            return;
        }
        ConsoleGFX::showLoading("Creating file", 500);
        if (current->files.count(name)) {
            std::cout << ConsoleGFX::YELLOW << "File '" << name << "' already exists." << ConsoleGFX::RESET << std::endl;
            return;
        }
        current->files[name] = new File(name);
        std::cout << ConsoleGFX::GREEN << "File '" << name << "' created." << ConsoleGFX::RESET << std::endl;
    }

    void writeFile(const std::string& name, const std::string& content) {
        ConsoleGFX::showLoading("Writing to file", 700);
        if (!current->files.count(name)) {
            std::cout << ConsoleGFX::RED << "File '" << name << "' does not exist." << ConsoleGFX::RESET << std::endl;
            return;
        }
        current->files[name]->content = content;
        std::cout << ConsoleGFX::GREEN << "Content written to file '" << name << "'." << ConsoleGFX::RESET << std::endl;
    }

    void readFile(const std::string& name) {
         ConsoleGFX::showLoading("Reading file", 300);
        if (!current->files.count(name)) {
            std::cout << ConsoleGFX::RED << "File '" << name << "' does not exist." << ConsoleGFX::RESET << std::endl;
            return;
        }
        std::cout << ConsoleGFX::BOLD_WHITE << "Content of '" << name << "':" << ConsoleGFX::RESET << std::endl;
        std::cout << current->files[name]->content << std::endl;
    }

    void deleteFile(const std::string& name) {
         ConsoleGFX::showLoading("Deleting file", 600);
        if (!current->files.count(name)) {
            std::cout << ConsoleGFX::RED << "File '" << name << "' does not exist." << ConsoleGFX::RESET << std::endl;
            return;
        }
        delete current->files[name];
        current->files.erase(name);
        std::cout << ConsoleGFX::GREEN << "File '" << name << "' deleted." << ConsoleGFX::RESET << std::endl;
    }

    void deleteDirectoryByName(const std::string& name) {
         if (name.empty() || name.find('/') != std::string::npos || name == "." || name == "..") {
            std::cout << ConsoleGFX::RED << "Invalid directory name for deletion." << ConsoleGFX::RESET << std::endl;
            return;
        }
        ConsoleGFX::showLoading("Deleting directory", 800);
        if (!current->subdirs.count(name)) {
            std::cout << ConsoleGFX::RED << "Directory '" << name << "' does not exist." << ConsoleGFX::RESET << std::endl;
            return;
        }
        deleteDirectoryRecursive(current->subdirs[name]); // Use the recursive helper
        current->subdirs.erase(name);
        std::cout << ConsoleGFX::GREEN << "Directory '" << name << "' deleted." << ConsoleGFX::RESET << std::endl;
    }

    void listDirectory() {
        ConsoleGFX::showLoading("Reading directory contents", 300);
        std::cout << ConsoleGFX::BOLD_WHITE << "Contents of " << getCurrentPath() << ":" << ConsoleGFX::RESET << std::endl;

        bool empty = true;

        if (!current->subdirs.empty()) {
             empty = false;
            std::cout << ConsoleGFX::BOLD_BLUE << "Directories:" << ConsoleGFX::RESET << std::endl;
            // Sort for consistent listing
            std::vector<std::string> subdir_names;
            for(const auto& pair : current->subdirs) subdir_names.push_back(pair.first);
            std::sort(subdir_names.begin(), subdir_names.end());
            for (const auto& name : subdir_names) {
                std::cout << ConsoleGFX::BLUE << " [D] " << ConsoleGFX::RESET << name << std::endl; // Indicate directory
            }
        }

        if (!current->files.empty()) {
             empty = false;
            std::cout << ConsoleGFX::BOLD_GREEN << "Files:" << ConsoleGFX::RESET << std::endl;
             // Sort for consistent listing
            std::vector<std::string> file_names;
            for(const auto& pair : current->files) file_names.push_back(pair.first);
            std::sort(file_names.begin(), file_names.end());
            for (const auto& name : file_names) {
                std::cout << ConsoleGFX::GREEN << " [F] " << ConsoleGFX::RESET << name << std::endl; // Indicate file
            }
        }

        if (empty) {
             std::cout << ConsoleGFX::YELLOW << "(empty)" << ConsoleGFX::RESET << std::endl;
        }
         std::cout << std::endl;
    }

    void changeDirectory(const std::string& name) {
         ConsoleGFX::showLoading("Changing directory", 400);
        if (name == "..") {
            if (current->parent) {
                current = current->parent;
                std::cout << ConsoleGFX::GREEN << "Changed directory to " << getCurrentPath() << ConsoleGFX::RESET << std::endl;
            } else {
                std::cout << ConsoleGFX::YELLOW << "Already at root directory." << ConsoleGFX::RESET << std::endl;
            }
        } else if (current->subdirs.count(name)) {
            current = current->subdirs.at(name); // Use .at() for bounds checking if needed, or just []
            std::cout << ConsoleGFX::GREEN << "Changed directory to " << getCurrentPath() << ConsoleGFX::RESET << std::endl;
        } else {
            std::cout << ConsoleGFX::RED << "Directory '" << name << "' not found." << ConsoleGFX::RESET << std::endl;
        }
    }

     // Public function to display the tree starting from root
    void displayTree() const {
        ConsoleGFX::showLoading("Generating tree", 500);
        std::cout << ConsoleGFX::BOLD_WHITE << "Directory Tree:" << ConsoleGFX::RESET << std::endl;
        std::cout << ConsoleGFX::BOLD_BLUE << "/" << ConsoleGFX::RESET << std::endl; // Print root manually

        // Print children of the root
        // Need to collect children first to determine the last sibling for the initial calls
         std::vector<std::pair<std::string, Directory*>> root_subdirs;
        for(const auto& pair : root->subdirs) root_subdirs.push_back(pair);
        std::sort(root_subdirs.begin(), root_subdirs.end(), [](const auto& a, const auto& b){ return a.first < b.first; });

        std::vector<std::pair<std::string, File*>> root_files;
        for(const auto& pair : root->files) root_files.push_back(pair);
        std::sort(root_files.begin(), root_files.end(), [](const auto& a, const auto& b){ return a.first < b.first; });

        size_t total_root_items = root_subdirs.size() + root_files.size();
        size_t current_root_item_index = 0;

        for (const auto& pair : root_subdirs) {
            current_root_item_index++;
            bool is_last_item = (current_root_item_index == total_root_items);
             // Start recursive call for child directories from root (initial prefix is empty)
            printDirectoryTreeRecursive(pair.second, "", is_last_item);
        }

        for (const auto& pair : root_files) {
             current_root_item_index++;
             bool is_last_item = (current_root_item_index == total_root_items);
             // Print files directly under root
             std::cout << (is_last_item ? "`-- " : "|-- ") << ConsoleGFX::GREEN << pair.first << ConsoleGFX::RESET << std::endl;
        }

        std::cout << std::endl;
    }


    std::string getCurrentPath() {
        std::vector<std::string> path_parts;
        Directory* temp = current;
        while (temp && temp != root) { // Stop when reaching the root
            path_parts.push_back(temp->name);
            temp = temp->parent;
        }
        std::string fullPath = "/"; // Start with the root slash
        for (auto it = path_parts.rbegin(); it != path_parts.rend(); ++it) {
            fullPath += *it + "/";
        }
        if (fullPath.size() > 1 && fullPath.back() == '/') {
             fullPath.pop_back(); // Remove trailing slash unless it's just "/"
        }
         if (fullPath.empty()) return "/"; // Should not happen with logic above, but as a safeguard
        return fullPath;
    }
};


//-----------------------------------------------------------------------------
// User Interface Functions
//-----------------------------------------------------------------------------

void displayWelcomeScreen() {
    ConsoleGFX::clearScreen(); // Clear screen before showing welcome
    std::cout << ConsoleGFX::BOLD_CYAN << R"(
*****************************************************
* *                                           *
* Welcome to the Virtual File System          *
* By Muhammad Zohaib                          *
* *                                           *
*****************************************************
)" << ConsoleGFX::RESET << std::endl;
    std::cout << ConsoleGFX::YELLOW << "Initializing..." << ConsoleGFX::RESET << std::endl;
    ConsoleGFX::showLoading("Starting up", 800); // Short startup animation
    std::cout << ConsoleGFX::BOLD_GREEN << "System Ready!" << ConsoleGFX::RESET << std::endl << std::endl;
    // Platform-dependent sleep
    #ifdef _WIN32
        Sleep(1000); // Pause briefly (1 second)
    #else
        usleep(static_cast<useconds_t>(1000 * 1000)); // Pause briefly (1 second)
    #endif
}

void displayMenu(const std::string& currentPath) {
    (void)currentPath;
    std::cout << std::endl; // Space before menu
cout << ConsoleGFX::CYAN;
cout << "\n+--------------------------------------------+\n";
cout << "|         Virtual File System Menu           |\n";
cout << "+--------------------------------------------+\n";
cout << ConsoleGFX::GREEN;
cout << "| 1. List Directory                          |\n";
cout << "| 2. Create Directory                        |\n";
cout << "| 3. Read File                               |\n";
cout << "| 4. Write to File                           |\n";
cout << "| 5. Read File                               |\n";
cout << "| 6. Delete File                             |\n";
cout << "| 7. Delete Directory                        |\n";
cout << "| 8. Change Directory                        |\n";
cout << "| 9. Display Tree                            |\n";
cout << "| 10.Exit                                   |\n";
cout << ConsoleGFX::CYAN;
cout << "+--------------------------------------------+\n";
cout << ConsoleGFX::BOLD_YELLOW << "Enter your choice: " << ConsoleGFX::RESET;

}

// Function to get a valid integer choice from the user
int getMenuChoice() {
    int choice = -1;
    std::cin >> choice;

    // Basic input validation
    while (std::cin.fail()) {
        std::cout << ConsoleGFX::RED << "Invalid input. Please enter a number." << ConsoleGFX::RESET << std::endl;
        std::cin.clear(); // Clear error flags
        // Discard invalid input from buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << ConsoleGFX::BOLD_YELLOW << "Enter your choice: " << ConsoleGFX::RESET;
        std::cin >> choice;
    }
     // Discard any leftover characters in the buffer (like the newline)
     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

// Function to get a string input (like file/dir name)
std::string getStringInput(const std::string& prompt) {
    std::string input;
    std::cout << ConsoleGFX::BOLD_YELLOW << prompt << ConsoleGFX::RESET;
    std::getline(std::cin, input); // Use getline to allow spaces in names
    return input;
}

// Function to get content input (multiline allowed until a sentinel)
std::string getContentInput(const std::string& prompt) {
    std::string content = "";
    std::string line;
    std::cout << ConsoleGFX::BOLD_YELLOW << prompt << ConsoleGFX::RESET << ConsoleGFX::YELLOW << " (Enter a single '.' on a new line to finish input):" << ConsoleGFX::RESET << std::endl;
    while (true) {
        std::getline(std::cin, line);
        if (line == ".") {
            break; // Sentinel to end input
        }
        content += line + "\n";
    }
    // Remove the last newline character if content is not empty
    if (!content.empty()) {
        content.pop_back();
    }
    return content;
}


//-----------------------------------------------------------------------------
// Main Application Logic
//-----------------------------------------------------------------------------
void launch_filesystem() {
    displayWelcomeScreen();

    VirtualFileSystem vfs; // Create the VFS object

    int choice;
    bool running = true;

    while (running) {
        ConsoleGFX::clearScreen(); // Clear screen at the start of each loop
        displayMenu(vfs.getCurrentPath()); // Pass current path to menu
        choice = getMenuChoice();

        std::string name, content; // For operations needing name/content

        switch (choice) {
            case 1: // List Directory
                vfs.listDirectory();
                break;
            case 2: // Create Directory
                name = getStringInput("Enter directory name: ");
                if (!name.empty()) vfs.createDirectory(name);
                else std::cout << ConsoleGFX::RED << "Directory name cannot be empty." << ConsoleGFX::RESET << std::endl;
                break;
            case 3: // Create File
                name = getStringInput("Enter file name: ");
                 if (!name.empty()) vfs.createFile(name);
                 else std::cout << ConsoleGFX::RED << "File name cannot be empty." << ConsoleGFX::RESET << std::endl;
                break;
            case 4: // Write to File
                 name = getStringInput("Enter file name to write to: ");
                 if (!name.empty()) {
                     content = getContentInput("Enter content");
                     vfs.writeFile(name, content);
                 } else {
                     std::cout << ConsoleGFX::RED << "File name cannot be empty." << ConsoleGFX::RESET << std::endl;
                 }
                break;
            case 5: // Read File
                 name = getStringInput("Enter file name to read: ");
                 if (!name.empty()) vfs.readFile(name);
                 else std::cout << ConsoleGFX::RED << "File name cannot be empty." << ConsoleGFX::RESET << std::endl;
                break;
            case 6: // Delete File
                 name = getStringInput("Enter file name to delete: ");
                 if (!name.empty()) vfs.deleteFile(name);
                  else std::cout << ConsoleGFX::RED << "File name cannot be empty." << ConsoleGFX::RESET << std::endl;
                break;
            case 7: // Delete Directory
                 name = getStringInput("Enter directory name to delete: ");
                 if (!name.empty()) vfs.deleteDirectoryByName(name);
                  else std::cout << ConsoleGFX::RED << "Directory name cannot be empty." << ConsoleGFX::RESET << std::endl;
                break;
            case 8: // Change Directory
                 name = getStringInput("Enter directory name (.. to go back): ");
                 if (!name.empty()) vfs.changeDirectory(name);
                 else std::cout << ConsoleGFX::RED << "Directory name cannot be empty." << ConsoleGFX::RESET << std::endl;
                break;
            case 9: // Display Tree
                 vfs.displayTree();
                 break;
            // case 10: // Clear Screen
            //     ConsoleGFX::clearScreen();
            //     break;
            case 10: // Exit
                running = false;
                system("clear");
                std::cout << ConsoleGFX::BOLD_CYAN << "\nExiting Virtual File System. Goodbye!" << ConsoleGFX::RESET << std::endl;
                break;
            default:
                std::cout << ConsoleGFX::RED << "Invalid choice. Please try again." << ConsoleGFX::RESET << std::endl;
                break;
        }

        if (running && choice != 10 && choice != 0) { // Pause slightly after an action (except clear/exit)
             std::cout << "\n" << ConsoleGFX::YELLOW << "(Press Enter to continue...)" << ConsoleGFX::RESET;
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Wait for user press Enter
        } else if (choice == 10) {
             // If only clear screen was chosen, don't pause, just loop
        }
    }

}