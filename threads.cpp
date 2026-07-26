// thread_sync_module - Final Version with Visual Enhancements and Beep
// g++ -std=c++20 -pthread thread_sync_module.cpp -o thread_sync_module

#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

#define RESET  "\033[0m"
#define GREEN  "\033[1;32m"
#define CYAN   "\033[1;36m"
#define YELLOW "\033[1;33m"
#define RED    "\033[1;31m"
#define BOLD   "\033[1m"
#define BEEP   '\a'

void loadingBar(const std::string& msg, int length = 20, int totalDurationMs = 1000) {
    std::cout << CYAN << msg << " [";
    for (int i = 0; i < length; ++i) {
        std::cout << "█" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(totalDurationMs / length));
    }
    std::cout << "] 100%" << RESET << std::endl;
    std::cout << BEEP << std::flush;
}

class SharedResource {
private:
    int data;
    std::mutex mtx;
    std::binary_semaphore sem{1};

public:
    SharedResource() : data(0) {}
    void writeWithMutex(int value, int id);
    int readWithMutex(int id);
    void accessWithSemaphore(int threadId);
};

void SharedResource::writeWithMutex(int value, int id) {
    loadingBar("[Mutex][Thread " + std::to_string(id) + "] Acquiring lock to write");
    std::lock_guard<std::mutex> lock(mtx);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << GREEN << "[Mutex][Thread " << id << "][Time: " << std::ctime(&now) << "] Writing data: " << value << RESET << std::endl;
    data = value;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << BEEP << std::flush;
}

int SharedResource::readWithMutex(int id) {
    loadingBar("[Mutex][Thread " + std::to_string(id) + "] Acquiring lock to read");
    std::lock_guard<std::mutex> lock(mtx);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << GREEN << "[Mutex][Thread " << id << "][Time: " << std::ctime(&now) << "] Reading data: " << data << RESET << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << BEEP << std::flush;
    return data;
}

void SharedResource::accessWithSemaphore(int threadId) {
    loadingBar("[Semaphore][Thread " + std::to_string(threadId) + "] Waiting for access");
    sem.acquire();
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << CYAN << "[Semaphore][Thread " << threadId << "][Time: " << std::ctime(&now) << "] Accessing resource." << RESET << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << CYAN << "[Semaphore][Thread " << threadId << "] Done." << RESET << std::endl;
    sem.release();
    std::cout << BEEP << std::flush;
}

class IPCChannel {
private:
    int message;
    bool ready;
    std::mutex mtx;
    std::condition_variable cv;

public:
    IPCChannel() : message(0), ready(false) {}
    void send(int msg);
    int receive();
};

void IPCChannel::send(int msg) {
    loadingBar("[IPC][Producer] Sending message");
    std::unique_lock<std::mutex> lock(mtx);
    message = msg;
    ready = true;
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << YELLOW << "[IPC][Producer][Time: " << std::ctime(&now) << "] Message sent: " << msg << RESET << std::endl;
    cv.notify_one();
    std::cout << BEEP << std::flush;
}

int IPCChannel::receive() {
    loadingBar("[IPC][Consumer] Waiting to receive");
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { return ready; });
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << YELLOW << "[IPC][Consumer][Time: " << std::ctime(&now) << "] Message received: " << message << RESET << std::endl;
    ready = false;
    std::cout << BEEP << std::flush;
    return message;
}

void threadWithMutex(SharedResource& resource, int id) {
    resource.writeWithMutex(id * 10, id);
    resource.readWithMutex(id);
}

void threadWithSemaphore(SharedResource& resource, int id) {
    resource.accessWithSemaphore(id);
}

void ipcProducer(IPCChannel& channel) {
    for (int i = 1; i <= 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        channel.send(i * 100);
    }
}

void ipcConsumer(IPCChannel& channel) {
    for (int i = 1; i <= 3; ++i) {
        channel.receive();
    }
}

void showMenu() {
    while (true) {
        system("clear");
        std::cout << RED << BOLD << "+----------------------------------------------+" << RESET << std::endl;
        std::cout << RED << BOLD << "|     Thread Synchronization and IPC Module    |" << RESET << std::endl;
        std::cout << RED << BOLD << "+----------------------------------------------+" << RESET << std::endl;

        int choice;

        std::cout << CYAN << BOLD;
        std::cout << "+------------------------------------------+" << std::endl;
        std::cout << "|           Simulation Menu                |" << std::endl;
        std::cout << "+------------------------------------------+" << std::endl;
        std::cout << "| 1. Mutex Synchronization                 |" << std::endl;
        std::cout << "| 2. Semaphore Synchronization             |" << std::endl;
        std::cout << "| 3. Thread-based IPC (Producer-Consumer)  |" << std::endl;
        std::cout << "| 4. Run All                               |" << std::endl;
        std::cout << "| 5. Exit                                  |" << std::endl;
        std::cout << "+------------------------------------------+" << std::endl;
        std::cout << "Enter choice: ";
        std::cin >> choice;

        SharedResource resource;
        IPCChannel channel;
        std::vector<std::thread> threads;

        system("clear");
        std::cout << GREEN << BOLD << "\n+-----------------------------+" << RESET << std::endl;
        std::cout << GREEN << BOLD <<   "|   Simulation In Progress   |" << RESET << std::endl;
        std::cout << GREEN << BOLD <<   "+-----------------------------+" << RESET << std::endl;

        switch (choice) {
            case 1:
                threads.emplace_back(threadWithMutex, std::ref(resource), 1);
                threads.emplace_back(threadWithMutex, std::ref(resource), 2);
                threads.emplace_back(threadWithMutex, std::ref(resource), 5);
                break;
            case 2:
                threads.emplace_back(threadWithSemaphore, std::ref(resource), 3);
                threads.emplace_back(threadWithSemaphore, std::ref(resource), 4);
                threads.emplace_back(threadWithSemaphore, std::ref(resource), 6);
                break;
            case 3:
                threads.emplace_back(ipcProducer, std::ref(channel));
                threads.emplace_back(ipcConsumer, std::ref(channel));
                break;
            case 4:
                threads.emplace_back(threadWithMutex, std::ref(resource), 1);
                threads.emplace_back(threadWithMutex, std::ref(resource), 2);
                threads.emplace_back(threadWithMutex, std::ref(resource), 5);
                threads.emplace_back(threadWithSemaphore, std::ref(resource), 3);
                threads.emplace_back(threadWithSemaphore, std::ref(resource), 4);
                threads.emplace_back(threadWithSemaphore, std::ref(resource), 6);
                threads.emplace_back(ipcProducer, std::ref(channel));
                threads.emplace_back(ipcConsumer, std::ref(channel));
                break;
            case 5:
                std::cout << GREEN << "\n[+] Exiting simulator. Goodbye!" << RESET << std::endl;
                return;
            default:
                std::cout << RED << "Invalid choice. Try again." << RESET << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
        }

        for (auto& t : threads) {
            t.join();
        }

        std::cout << GREEN << "\n[+] Simulation completed successfully." << RESET << std::endl;
        std::cout << "\nPress Enter to return to main menu...";
        std::cin.ignore();
        std::cin.get();
    }
}

void launch_threads() {
    showMenu();
}

