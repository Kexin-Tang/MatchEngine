#include "bookEntry.h"
#include "order.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <map>
#include <deque>

class MatchEngine {
private:
    std::unordered_map<std::string, Order>                  orders_;
    std::map<int, std::deque<BookEntry>, std::greater<int>> buyBook_;
    std::map<int, std::deque<BookEntry>>                    sellBook_;
    long long                                               timestamp_ = 0;

    // match
    void tryToMatch(Order& o);
    void matchBuy(Order& o);
    void matchSell(Order& o);

    // valid
    bool isValidEntry(const BookEntry& entry, Side side) const;

    // print transaction when there is a match
    void printTransaction(const Order& lhs, const Order& rhs, int qty) const;

public:
    MatchEngine() = default;
    ~MatchEngine() = default;

    // supported commands
    void buy(Type type, int price, int qty, const std::string& id);
    void sell(Type type, int price, int qty, const std::string& id);
    void cancel(const std::string& id);
    void modify(const std::string& id, Side newSide, int newPrice, int newQty);
    void print() const;
};
