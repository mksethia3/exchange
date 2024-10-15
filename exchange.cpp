#include "exchange.h"
#include "matchingengine.h"
#include "listener.h"
#include "broadcaster.h"

#include <iostream>
#include <iomanip>

/* Constructor of Exchange, create new ME, listener, broadcaster in unique_ptrs, 
start listener & broadcaster threads*/
Exchange::Exchange(int numSymbols, int inPort) 
    : running(true), inOrderCount(0), outOrderCount(0) 
{
    matchingEngine = std::make_unique<MatchingEngine>(numSymbols);

    listener = std::make_unique<Listener>(
        inQueue, 
        inQueueMutex, 
        inQueueCV, 
        inPort
    );
    broadcaster = std::make_unique<Broadcaster>(
        outQueue, 
        outQueueMutex, 
        outQueueCV, 
        // outPort,
        listener->getClientSockets(),
        listener->getClientSocketsMutex()
    );

    broadcasterThread = std::thread([this](){broadcaster->run();});
    listenerThread = std::thread([this](){listener->run();});
    infoThread = std::thread([this](){updateDisplay();});

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

Exchange::~Exchange() {
    stop();
    listenerThread.join();
    broadcasterThread.join();
    infoThread.join();
}




/* Process orders, called forever by run() after run() called by main 
Process orders empties queue once called */
void Exchange::stop() {
    running = false;
    listener->stop();
    broadcaster->stop();
    inQueueCV.notify_all();
    outQueueCV.notify_all();
}

void Exchange::run() {
    //listener->run();
    //broadcaster->run();
    while (running) {
        // std::cout << "Processing orders ... \n";
        processOrders();
        // std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}



void Exchange::processOrders() {
    std::pair<int, Order> orderPair;
    {
        std::unique_lock<std::mutex> lock(inQueueMutex);
        inQueueCV.wait(lock, [this](){return !inQueue.empty() || !running;});
        if (!running || inQueue.empty()) return;
        orderPair = inQueue.front();
        inQueue.pop();
        inOrderCount++;
    }

    ExecStatus result = matchingEngine->execOrder(orderPair.first, orderPair.second);
    {
        std::lock_guard<std::mutex> lock(outQueueMutex);
        outQueue.push(result);
        outOrderCount++;
    }

    outQueueCV.notify_one();
}

void Exchange::updateDisplay() {
    using namespace std::chrono;
    auto lastUpdate = steady_clock::now();

    std::cout << "Incoming Orders/sec: 0 | Outgoing Orders/sec: 0" << std::flush;

    while (running) {
        auto now = steady_clock::now();
        if (duration_cast<milliseconds>(now - lastUpdate) >= milliseconds(200)) {
            int incoming = inOrderCount.exchange(0) * 5;
            int outgoing = outOrderCount.exchange(0) * 5;

            std::cout << "\rIncoming Orders/sec: " << std::setw(6) << incoming 
                      << " | Outgoing Orders/sec: " << std::setw(6) << outgoing << std::flush;

            lastUpdate = now;
        }
        // std::this_thread::sleep_for(milliseconds(10));
    }
    std::cout << "\n";
}