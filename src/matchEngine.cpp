#include "matchEngine.h"
#include <iostream>
#include <stdexcept>

// =============== private ===============

// validate entry
bool MatchEngine::isValidEntry(const BookEntry& entry, Side side) const {
    auto it =  orders_.find(entry.id);

    // cannot find the order relates to given order id
    if (it == orders_.end())
        return false;

    const Order& o = it->second;
    return o.isActive && o.timestamp == entry.timestamp && o.side == side && o.qty > 0;
};

// order match
void MatchEngine::tryToMatch(Order& o) {
    if (o.side == Side::BUY) {
        matchBuy(o);
    } else {
        matchSell(o);
    }
};

void MatchEngine::matchBuy(Order& o) {
    // try to match sell book entries
    while (o.qty > 0 && !sellBook_.empty()) {
        // clean all invalid entries from sell book
        auto it = sellBook_.begin();
        std::deque<BookEntry>& q = it->second;
        int sellPrice = it->first;
        while (!q.empty() && !isValidEntry(q.front(), Side::SELL)) {
            q.pop_front();
        }
        
        // if nothing left in the queue, try next
        if (q.empty()) {
            sellBook_.erase(it);
            continue;
        }

        // only execute the order when `buy price >= sell price`
        if (o.price < sellPrice)
            break;

        Order& sellOrder = orders_[q.front().id];

        // get how many qty is traded
        int tradeQty = std::min(o.qty, sellOrder.qty);

        printTransaction(sellOrder, o, tradeQty);

        o.qty -= tradeQty;
        sellOrder.qty -= tradeQty;

        // if current sell order has been fully traded
        // mark the order as inactive and pop this entry from sell book
        if (sellOrder.qty == 0) {
            sellOrder.isActive = false;
            q.pop_front();
        }
    }
};

void MatchEngine::matchSell(Order& o) {
    // try to match buy book entries
    while (o.qty > 0 && !buyBook_.empty()) {
        // clean all invalid entries from buy book
        auto it = buyBook_.begin();
        std::deque<BookEntry>& q = it->second;
        int buyPrice = it->first;
        while (!q.empty() && !isValidEntry(q.front(), Side::BUY)) {
            q.pop_front();
        }
        
        // if nothing left in the queue, try next
        if (q.empty()) {
            buyBook_.erase(it);
            continue;
        }

        // only execute the order when `buy price >= sell price`
        if (o.price > buyPrice)
            break;

        Order& buyOrder = orders_[q.front().id];

        // get how many qty is traded
        int tradeQty = std::min(o.qty, buyOrder.qty);

        printTransaction(buyOrder, o, tradeQty);

        o.qty -= tradeQty;
        buyOrder.qty -= tradeQty;

        // if current buy order has been fully traded
        // mark the order as inactive and pop this entry from buy book
        if (buyOrder.qty == 0) {
            buyOrder.isActive = false;
            q.pop_front();
        }
    }
};

// print transaction
void MatchEngine::printTransaction(const Order& lhs, const Order& rhs, int qty) const {
    const Order& first = lhs.timestamp < rhs.timestamp ? lhs : rhs;
    const Order& second = lhs.timestamp < rhs.timestamp ? rhs : lhs;

    std::cout << "TRADE "
              << first.id << " " << first.price << " " << qty << " "
              << second.id << " " << second.price << " " << qty << std::endl;
};

// =============== public ===============
void MatchEngine::buy(Type type, int price, int qty, const std::string &id) {
    if (price <= 0 || qty <= 0 || id.empty())
        throw std::runtime_error("Invalid order");

    if (orders_.count(id) && orders_[id].isActive)
        throw std::runtime_error("Order already exist");

    // create the new buy order
    Order o = Order(Side::BUY, type, price, qty, id, true, timestamp_);
    timestamp_++;
    
    // try to match sell orders
    tryToMatch(o);
    
    // if this order is fully traded
    if (o.qty <= 0) return;

    // if this order remains some qty, buy its type is IOC (buy partial and cancel the remaining)
    if (type == Type::IOC) return;

    // otherwise, add this order to orders and buy book
    orders_[o.id] = o;
    buyBook_[o.price].emplace_back(o.id, o.timestamp);
};

void MatchEngine::sell(Type type, int price, int qty, const std::string &id) {
    if (price <= 0 || qty <= 0 || id.empty())
        throw std::runtime_error("Invalid order");

    if (orders_.count(id) && orders_[id].isActive)
        throw std::runtime_error("Order already exist");

    // create the new sell order
    Order o = Order(Side::SELL, type, price, qty, id, true, timestamp_);
    timestamp_++;
    
    // try to match buy orders
    tryToMatch(o);
    
    // if this order is fully traded
    if (o.qty <= 0) return;

    // if this order remains some qty, buy its type is IOC (sell partial and cancel the remaining)
    if (type == Type::IOC) return;

    // otherwise, add this order to orders and sell book
    orders_[o.id] = o;
    sellBook_[o.price].emplace_back(o.id, o.timestamp);
};

void MatchEngine::cancel(const std::string &id) {
    if (orders_.count(id) == 0)
        throw std::runtime_error("Cannot find order");
    
    orders_[id].isActive = false;
};

void MatchEngine::modify(const std::string &id, Side newSide, int newPrice, int newQty) {
    if (newPrice <= 0 || newQty <= 0 || id.empty())
        throw std::runtime_error("Invalid order");

    if (orders_.count(id) == 0)
        throw std::runtime_error("Cannot find order");

    Order& originOrder = orders_[id];
    if (originOrder.isActive)
        throw std::runtime_error("Try to modify an inactive order");

    if (originOrder.type == Type::IOC)
        throw std::runtime_error("Cannot modify IOC order");

    // mark old order as inactive
    originOrder.isActive = false;

    // create a new order, then try to match orders
    Order newOrder = Order(newSide, Type::GFD, newPrice, newQty, id, true, timestamp_);
    timestamp_++;

    tryToMatch(newOrder);

    // if this order remains
    if (newOrder.qty > 0) {
        orders_[id] = newOrder;
        if (Side::BUY == newOrder.side) {
            buyBook_[newOrder.price].emplace_back(newOrder.id, newOrder.timestamp);
        } else {
            sellBook_[newOrder.price].emplace_back(newOrder.id, newOrder.timestamp);
        }
    }
};

void MatchEngine::print() const {
    std::cout << "SELL:" << std::endl;
    for (auto it = sellBook_.rbegin(); it != sellBook_.rend(); ++it) {
        long long totalQty = 0;
        for (const auto& entry : it->second) {
            if (isValidEntry(entry, Side::SELL)) {
                    totalQty += orders_.at(entry.id).qty;
                }
            }
            if (totalQty > 0) {
                std::cout << it->first << " " << totalQty << std::endl;
            }

        }

    std::cout << "BUY:" << std::endl;
    for (const auto& [price, q] : buyBook_) {
        long long totalQty = 0;
        for (const auto& entry : q) {
            if (isValidEntry(entry, Side::BUY)) {
                totalQty += orders_.at(entry.id).qty;
            }
        }
        if (totalQty > 0) {
            std::cout << price << " " << totalQty << std::endl;
        }
    }
};
