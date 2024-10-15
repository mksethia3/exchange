#pragma once

#include <unordered_map>
#include <thread>

#include "matchingengine.h"
#include "listener.h"
#include "broadcaster.h"

class Exchange {
private:
    std::unique_ptr<MatchingEngine> matchingEngine;
    std::unique_ptr<Listener> listener;
    std::unique_ptr<Broadcaster> broadcaster;

    std::queue<std::pair<int, Order>> inQueue;
    std::queue<ExecStatus> outQueue;
    std::mutex inQueueMutex, outQueueMutex;
    std::condition_variable inQueueCV, outQueueCV;

    std::atomic<bool> running;
    std::thread listenerThread;
    std::thread broadcasterThread;
    std::thread infoThread;

    std::atomic<int> inOrderCount;
    std::atomic<int> outOrderCount;

public:
    Exchange(
        int numSymbols, int inPort /*, int outPort*/);
    ~Exchange();

    void run();
    void stop();

private:
    void processOrders();
    void updateDisplay();
};