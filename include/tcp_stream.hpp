#pragma once
#include "basic_network.hpp"

namespace Slate::Stream
{
    /*  
        Summary:
            Adds the tcp read and write functions to a stream type
    */
    template <typename Stream>
    class TCP : public Network
    {
    public:
        void Open(std::string const& address)
        {
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            connect(this->socket_fd, nullptr, 0);
        }
    };
}