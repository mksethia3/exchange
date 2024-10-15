#include "exchange.h"

#include <iostream>

#define NUM_SYMBOLS 256
#define IN_PORT 8080
#define OUT_PORT 8081

int main() {
    Exchange exchange(NUM_SYMBOLS, IN_PORT /*, OUT_PORT*/);
    exchange.run();
    exchange.stop();
    std::cout << "exchange succesfully started and stopped\n";
    return 0;
}