#pragma once

#include "orderbook.h"

#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <sys/socket.h>
#include <netinet/in.h>

class Listener {
private:
    std::queue<std::pair<int, Order>>& inQueue;
    std::mutex& inQueueMutex;
    std::condition_variable& inQueueCV;
    std::atomic<bool> running;

    int serverSocket;
    std::unordered_set<int> clientSockets;
    std::mutex clientSocketsMutex;

public:
    Listener(
        std::queue<std::pair<int, Order>>& _queue,
        std::mutex& _mutex,
        std::condition_variable& _cv,
        int inPort
    );
    ~Listener();

    void run();
    void stop();
    const std::unordered_set<int>& getClientSockets();
    std::mutex& getClientSocketsMutex();

private:
    void closeSocket(int socket);
    // void listen();
    void acceptNewConnections();
    void recieveOrders();
    std::pair<int, Order> parseOrder(const std::string& orderStr);
};

