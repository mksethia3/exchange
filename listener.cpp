#include "listener.h"

#include <iostream>
#include <thread>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

/* Constructor / destructor. constructor inits listening socket */
Listener::Listener(
    std::queue<std::pair<int, Order>>& _queue,
    std::mutex& _mutex,
    std::condition_variable& _cv,
    int inPort
) : inQueue(_queue), inQueueMutex(_mutex), inQueueCV(_cv), running(true) {

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw std::runtime_error("couldn't create socket");
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        throw std::runtime_error("couldn't setsockopt");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(inPort);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("couldn't bind listener socket");
    }

    if (listen(serverSocket, 10) < 0) {
        throw std::runtime_error("couldn't listen on socket");
    }

    // non blocking
    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
}

Listener::~Listener() {
    stop();
    for (int clientSocket : clientSockets) closeSocket(clientSocket);
    closeSocket(serverSocket);
}



/* start/stop */
void Listener::run() {
    // std::cout << "Listener thread started\n";
    while (running) {
        acceptNewConnections();
        recieveOrders();
        // std::cout << "listening...\n";
        // std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void Listener::stop() {
    running = false;
}


/* main functions of thread, called indefinitely */
void Listener::acceptNewConnections() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (newSocket >= 0) {
        // std::cout << "NEW CONNECTION\n";

        int flags = fcntl(newSocket, F_GETFL, 0);
        fcntl(newSocket, F_SETFL, flags | O_NONBLOCK);
        std::lock_guard<std::mutex> lock(clientSocketsMutex);
        clientSockets.insert(newSocket);
    }
}

void Listener::recieveOrders() {
    char buffer[1024];
    std::lock_guard<std::mutex> lock(clientSocketsMutex);

        for (auto it = clientSockets.begin(); it != clientSockets.end();) {
        int clientSocket = *it;
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead > 0) {
            // std::cout << "ORDER RECIEVED " << buffer << "\n";

            buffer[bytesRead] = '\0';
            std::pair<int, Order> order = parseOrder(buffer);
            {
                std::lock_guard<std::mutex> queueLock(inQueueMutex);
                inQueue.push(order);
            }
            inQueueCV.notify_one();
            it++;
        } else if (bytesRead == 0 || (bytesRead == -1 && errno != EWOULDBLOCK)) {
            closeSocket(clientSocket);
            it = clientSockets.erase(it);
        } else {
            it++;
        }
    }
}



/* Helpers */
std::pair<int, Order> Listener::parseOrder(const std::string& orderStr) {
    std::istringstream iss(orderStr);
    int orderId, symbolId, qty;
    std::string orderTypeStr;
    double price;

    iss >> orderId >> symbolId >> orderTypeStr >> price >> qty;

    Order::OrderType orderType;
    if (orderTypeStr == "LIMIT_BUY") {
        orderType = Order::OrderType::LIMIT_BUY;
    } else if (orderTypeStr == "LIMIT_SELL") {
        orderType = Order::OrderType::LIMIT_SELL;
    } else if (orderTypeStr == "CANCEL") {
        orderType = Order::OrderType::CANCEL;
    } else {
        std::cout << orderStr << "\n";
        throw std::runtime_error("invalid order type");
    }

    return std::make_pair(symbolId, Order(orderId, orderType, price, qty));
}

const std::unordered_set<int>& Listener::getClientSockets() {
    return clientSockets;
}
std::mutex& Listener::getClientSocketsMutex() {
    return clientSocketsMutex;
}
void Listener::closeSocket(int socket) {
    if (shutdown(socket, SHUT_RDWR) < 0) {
        std::cerr << "error in shutting down socket";
    }
    if (close(socket) < 0) {
        std::cerr << "error in closing socket";
    }
}