#pragma once

#include <atomic>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <condition_variable>

#include "orderbook.h"

class Broadcaster {
private:
    std::queue<ExecStatus>& outQueue;
    std::mutex& outQueueMutex;
    std::condition_variable& outQueueCV;
    std::atomic<bool> running;

    const std::unordered_set<int>& clientSockets;
    std::mutex& clientSocketsMutex;

public:
    Broadcaster(
        std::queue<ExecStatus>& _queue, 
        std::mutex& _mutex, 
        std::condition_variable& _cv,
        // int outPort,

        const std::unordered_set<int>& _clientSockets,
        std::mutex& _clientSocketsMutex
    );
    ~Broadcaster();

    void run();
    void stop();

private:
    void broadcast();
    std::string serialiseExecStatus(const ExecStatus& status);
};