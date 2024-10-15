#pragma once

#include "orderbook.h"

class MatchingEngine {
private:
    std::vector<OrderBook> orderBooks;

public:
    MatchingEngine(int numSymbols);
    ExecStatus execOrder(int symbolId, Order& o);
};