#include "broadcaster.h"

#include <sstream>
#include <iostream>
#include <thread>
#include <sys/socket.h>


/* Constructor & Destructor */
Broadcaster::Broadcaster(
    std::queue<ExecStatus>& _queue, 
    std::mutex& _mutex, 
    std::condition_variable& _cv,
    // int outPort,

    const std::unordered_set<int>& _clientSockets,
    std::mutex& _clientSocketsMutex)
    : 
    outQueue(_queue), 
    outQueueMutex(_mutex), 
    outQueueCV(_cv), 
    running(true), 
    clientSockets(_clientSockets),
    clientSocketsMutex(_clientSocketsMutex) {}

Broadcaster::~Broadcaster() {
    stop();
}



/* start/stop */
void Broadcaster::run() {
    while (running) {
        // std::cout << "broadcasting ...\n";
        broadcast();
        // std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void Broadcaster::stop() {
    running = false;
}



/* Broadcast func, called repeatedly by broadcaster thread */
void Broadcaster::broadcast() {
    ExecStatus execStatus;
    {
        std::unique_lock<std::mutex> lock(outQueueMutex);
        outQueueCV.wait(lock, [this](){return !outQueue.empty() || !running;});
        if (!running || outQueue.empty()) return;
        execStatus = outQueue.front();
        outQueue.pop();
    }

    std::string msg = serialiseExecStatus(execStatus);

    {
        std::lock_guard<std::mutex> lock(clientSocketsMutex);
        for (int clientSocket : clientSockets) {

            // std::cout << "SENT " << msg << "\n";

            send(clientSocket, msg.c_str(), msg.length(), 0);
        }
    }
}

std::string Broadcaster::serialiseExecStatus(const ExecStatus& status) {
    std::ostringstream oss;
    oss << status.orderId << " " 
        << (status.success ? "SUCCESS" : "FAILURE") << " "
        << status.unfilledQty << " "
        << status.time;
    return oss.str();
}