#include <string>

// Buy / Sell
enum class Side { BUY, SELL };

// GFD / IOC
enum class Type { GFD, IOC };

// Order
struct Order {
    Side side;
    Type type;
    int price;
    int qty;
    std::string id;
    bool isActive;
    long long timestamp;
    
    Order() = default;
    ~Order() = default;
    
    Order(
        Side side,
        Type type,
        int price,
        int qty,
        const std::string& id,
        bool isActive,
        long long ts
    ) : side(side),
        type(type),
        price(price),
        qty(qty),
        id(id),
        isActive(isActive),
        timestamp(ts) {}
};
