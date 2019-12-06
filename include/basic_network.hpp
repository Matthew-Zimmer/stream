#pragma once
#include <string>
#include <vector>

#include <sys/socket.h>//linux only :P
#include <unistd.h>

namespace Slate::Streams
{
    /*  
        Summary:
            Adds the tcp read and write functions to a stream type
    */
    class Network
    {
    public:
        int socket_fd;
        ~Network()
        {
            if (socket_fd)
                close(socket_fd);
        }
        template <typename Type>
        auto Read() -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, Type>
        {
            Type t;
            recv(socket_fd, reinterpret_cast<void*>(&t), sizeof(t), 0);
            return t;
        }

        std::vector<char> Read(std::size_t size)
        {
            std::vector<char> bytes(size);
            recv(socket_fd, reinterpret_cast<void*>(bytes.data()), size, 0);
            return bytes;
        }

        template <typename Type>
        auto Write(Type t) -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, Type>
        {
            send(socket_fd, reinterpret_cast<void const*>(&t), sizeof(t), 0);
        }

        void Write(std::vector<char>& bytes)
        {
            send(socket_fd, bytes.data(), bytes.size(), 0);
        }
    };
}