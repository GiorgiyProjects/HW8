#include "async.h"

namespace async {

handle_t connect(std::size_t bulk) {
    return new MultithreadCommandParser(bulk);
}

void receive(handle_t handle, const char *data, [[maybe_unused]]std::size_t size) {
    std::string in = data;
    MultithreadCommandParser* M = reinterpret_cast<MultithreadCommandParser*>(handle);
    M->ReceiveInput(in);
    return;
}

void disconnect(handle_t handle) {
    delete reinterpret_cast<MultithreadCommandParser*>(handle);
}

}
