#include "src/matchEngine.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

Type parseType(const std::string& token) {
    if (token == "GFD") {
        return Type::GFD;
    }
    if (token == "IOC") {
        return Type::IOC;
    }
    throw std::runtime_error("Invalid order type");
}

Side parseSide(const std::string& token) {
    if (token == "BUY") {
        return Side::BUY;
    }
    if (token == "SELL") {
        return Side::SELL;
    }
    throw std::runtime_error("Invalid side");
}

} // namespace

int main() {
    MatchEngine engine;
    std::ifstream input("input.txt");
    if (!input.is_open()) {
        std::cerr << "Failed to open input.txt\n";
        return 1;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        try {
            if (cmd == "BUY") {
                std::string typeToken;
                int price = 0;
                int qty = 0;
                std::string id;
                if (iss >> typeToken >> price >> qty >> id) {
                    engine.buy(parseType(typeToken), price, qty, id);
                }
            } else if (cmd == "SELL") {
                std::string typeToken;
                int price = 0;
                int qty = 0;
                std::string id;
                if (iss >> typeToken >> price >> qty >> id) {
                    engine.sell(parseType(typeToken), price, qty, id);
                }
            } else if (cmd == "CANCEL") {
                std::string id;
                if (iss >> id) {
                    engine.cancel(id);
                }
            } else if (cmd == "MODIFY") {
                std::string id;
                std::string sideToken;
                int newPrice = 0;
                int newQty = 0;
                if (iss >> id >> sideToken >> newPrice >> newQty) {
                    engine.modify(id, parseSide(sideToken), newPrice, newQty);
                }
            } else if (cmd == "PRINT") {
                engine.print();
            }
        } catch (const std::exception&) {
            // Ignore invalid commands and failed operations so processing can continue.
        }
    }

    return 0;
}
