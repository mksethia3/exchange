#include "orderbook.h"

/* ExecStatus, Order, OrderBook constructors */
ExecStatus::ExecStatus(
    int _orderId,
    int _success,
    int _unfilledQty,
    std::time_t _time
) {
    orderId = _orderId;
    success = _success;
    unfilledQty = _unfilledQty;
    time = _time;
}
Order::Order(
    int _orderId,
    OrderType _orderType,
    double _price,
    int _qty
) {
    orderId = _orderId;
    orderType = _orderType;
    price = _price;
    qty = _qty;
}
Order::Order(
    int _orderId,
    OrderType _orderType,
    int _timeToCancel
) {
    orderId = _orderId;
    orderType = _orderType;
    timeToCancel = _timeToCancel;
}
OrderBook::OrderBook() {}


/* buy, sell & cancel */
ExecStatus OrderBook::execBuy(Order& o) {

    // continously fill against best sell prices
    while (o.qty > 0 && !sellBook.empty() && sellBook.rbegin()->price <= o.price) {
        Order& bestSell = const_cast<Order&>(*sellBook.begin());

        int matchedQty = std::min(o.qty, bestSell.qty);
        o.qty -= matchedQty;
        bestSell.qty -= matchedQty;

        if (bestSell.qty == 0) sellBook.erase(sellBook.begin());
    }

    // done -> if our buy order wasn't fully filled, add remaining to buy book
    if (o.qty > 0) {
        buyIteratorOf[o.orderId] = buyBook.insert(o).first;
    }
    return ExecStatus(o.orderId, true, o.qty, std::time(nullptr));
}

ExecStatus OrderBook::execSell(Order& o) {

    // continuously fill against best buy prices
    while (o.qty > 0 && !buyBook.empty() && buyBook.begin()->price >= o.price) {
        Order& bestBuy = const_cast<Order&>(*buyBook.begin());

        int matchedQty = std::min(o.qty, bestBuy.qty);
        o.qty -= matchedQty;
        bestBuy.qty -= matchedQty;

        if (bestBuy.qty == 0) buyBook.erase(buyBook.begin());
    }

    // done -> if our sell order wasn't fully filled, add remaining to sell book
    if (o.qty > 0) {
        sellIteratorOf[o.orderId] = sellBook.insert(o).first;
    }
    return ExecStatus(o.orderId, true, o.qty, std::time(nullptr));
}

ExecStatus OrderBook::execCancel(Order& o) {
    auto it1 = buyIteratorOf.find(o.orderId);
    auto it2 = sellIteratorOf.find(o.orderId);

    if (it1 != buyIteratorOf.end()) {
        buyBook.erase(it1->second);
    }
    if (it2 != sellIteratorOf.end()) {
        sellBook.erase(it2->second);
    }

    return ExecStatus(o.orderId, true, -1, std::time(nullptr));
}
