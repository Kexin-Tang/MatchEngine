# Matching Engine

## Overview

This project implements a simplified exchange-style order matching engine in modern C++.

The engine reads commands from standard input, processes orders in real time, maintains an order book, matches executable trades, and prints trade/book output according to the specification.

The system supports common exchange operations such as submitting orders, cancelling existing orders, modifying resting orders, and printing the current aggregated book state.

---

## Supported Commands

### 1. BUY

Submit a buy order.

```text
BUY <order_type> <price> <quantity> <order_id>
```

Example:

```text
BUY GFD 1000 10 order1
```

### 2. SELL

Submit a sell order.

```text
SELL <order_type> <price> <quantity> <order_id>
```

Example:

```text
SELL IOC 900 5 order2
```

### 3. CANCEL

Cancel an existing active order.

```text
CANCEL <order_id>
```

If the order does not exist, the command is ignored.

### 4. MODIFY

Modify an existing active order.

```text
MODIFY <order_id> <BUY|SELL> <new_price> <new_quantity>
```

Notes:

- IOC orders cannot be modified.
- A modified order loses its previous time priority.

### 5. PRINT

Print the current aggregated order book.

```text
PRINT
```

Output format:

```text
SELL:
<price> <quantity>
...

BUY:
<price> <quantity>
...
```

---

## Order Types

### GFD (Good For Day)

- Remains in the order book until fully executed, cancelled, or modified.

### IOC (Immediate Or Cancel)

- Executes immediately against available liquidity.
- Any unfilled remainder is cancelled.
- Never rests on the book.

---

## Matching Rules

Orders are matched whenever prices cross.

### Buy Order

A buy order trades when:

```text
buy_price >= best_sell_price
```

### Sell Order

A sell order trades when:

```text
sell_price <= best_buy_price
```

---

## Priority Rules

The engine uses **Price-Time Priority**:

1. Better price first.
  - Higher bid price has priority for buys.
  - Lower ask price has priority for sells.
2. Earlier time first.
  - For orders at the same price level, earlier orders are matched first.

---

## Trade Output

Each successful execution prints:

```text
TRADE <order1_id> <price1> <qty> <order2_id> <price2> <qty>
```

The order processed earlier appears first.

---

## Validation Rules

Commands are ignored if they contain:

- price <= 0
- quantity <= 0
- empty order id
- duplicate active order id

---

## Input

Commands are read from standard input until EOF.

Example:

```text
BUY GFD 1000 10 order1
SELL GFD 900 5 order2
PRINT
```

---

## Build

Build from the project root with CMake:

```bash
cmake -S . -B build
cmake --build build
```

This creates the executable at `build/src/main`.

If you prefer a single `g++` command, compile all translation units:

```bash
g++ -std=c++20 -O2 -I./src main.cpp src/*.cpp -o engine
```

## Run

Using the CMake build:

```bash
./build/src/main < input.txt
```

Using the `g++` build:

```bash
./engine < input.txt
```

