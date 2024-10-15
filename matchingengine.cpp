#include "matchingengine.h"

MatchingEngine::MatchingEngine(int numSymbols) :
    orderBooks(numSymbols)
{}

ExecStatus MatchingEngine::execOrder(int symbolId, Order& order) {
    // check well formed order

    switch (order.orderType) {
        case (Order::OrderType::LIMIT_BUY):
            return orderBooks[symbolId].execBuy(order);
        case (Order::OrderType::LIMIT_SELL):
            return orderBooks[symbolId].execSell(order);
        case (Order::OrderType::CANCEL):
            return orderBooks[symbolId].execCancel(order);
        default:
            return ExecStatus(-1, -1, -1, -1);
    }
}

