#include "async.h"
#include <map>
#include <memory>

namespace async {
    // this map is created in order to be safe in case of lack of disconnect function
    std::map<handle_t,std::shared_ptr<MultithreadCommandParser>> mm;

handle_t connect(std::size_t bulk) {
    auto command_parser_ptr =  std::make_shared<MultithreadCommandParser>(bulk);
    mm.emplace(std::make_pair(command_parser_ptr.get(),command_parser_ptr));
    return command_parser_ptr.get();
}

void receive(handle_t handle, const char *data, [[maybe_unused]]std::size_t size) {
    std::string in = data;
    auto ptr = mm.find(handle);
    if (ptr!=mm.end()) {
        ptr->second->ReceiveInput(in);
    }
    return;
}

void disconnect(handle_t handle) {
    auto ptr = mm.find(handle);
    if (ptr!=mm.end()) {
        mm.erase(ptr);
    }
}

}
