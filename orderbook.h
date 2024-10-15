#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <ctime>

struct ExecStatus {
    int orderId;
    bool success;
    int unfilledQty;
    std::time_t time;

    ExecStatus(
        int _orderId,
        int _success,
        int _unfilledQty,
        std::time_t _time
    );

    // default
    ExecStatus() {}
};

struct Order { 
    int orderId;
    enum class OrderType {LIMIT_BUY, LIMIT_SELL, CANCEL} orderType;
    double price;
    int qty;
    int timeToCancel;

    // Cancel order
    Order(
        int _orderId,
        OrderType _orderType,
        int _timeToCancel
    );
    // Buy / Sell order
    Order(
        int _orderId,
        OrderType _orderType,
        double _price,
        int _qty
    );
    // Default
    Order() {}
};

class OrderBook {
private:
    struct BuyBookComparator {
        bool operator()(const Order& o1, const Order& o2) const {
            if (o1.price == o2.price) {
                return o1.orderId < o2.orderId;
            }
            return o1.price > o2.price;
        }
    };
    struct SellBookComparator {
        bool operator()(const Order& o1, const Order& o2) const {
            if (o1.price == o2.price) {
                return o1.orderId < o2.orderId;
            }
            return o1.price < o2.price;
        }
    };

public:
    using BuyBook = std::set<Order, BuyBookComparator>;
    using SellBook = std::set<Order, SellBookComparator>;

private:
    BuyBook buyBook;
    SellBook sellBook;
    std::unordered_map<int, BuyBook::iterator> buyIteratorOf;
    std::unordered_map<int, SellBook::iterator> sellIteratorOf;
    std::mutex orderBookMutex;

public:
    OrderBook();
    ExecStatus execBuy(Order& o);
    ExecStatus execSell(Order& o);
    ExecStatus execCancel(Order& o);
};