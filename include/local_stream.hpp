#pragma once
#include <fstream>
#include <string>
#include <vector>

namespace Slate::Stream
{
    /*  
        Summary:
            Adds the local read and write functions to a stream type
    */
    template <typename Stream>
    class Local
    {
        std::fstream stream;
    public:
        void Open(std::string const& filename)
        {
            stream.open(filename, stream.binary | stream.trunc | stream.in | stream.out);
        }

        template <typename Type>
        auto Read() -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, Type>
        {
            Type t;
            stream.read(reinterpret_cast<char*>(&t), sizeof(t));
            return t;
        }

        std::vector<char> Read(std::size_t size)
        {
            std::vector<char> bytes(size);
            stream.read(reinterpret_cast<char*>(bytes.data()), size);
            return bytes;
        }

        template <typename Type>
        auto Write(Type t) -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, Type>
        {
            stream.write(reinterpret_cast<char*>(&t), sizeof(t));
        }

        void Write(std::vector<char>& bytes)
        {
            stream.write(bytes.data(), bytes.size());
        }

        void Seek()
        {
            stream.seekg(0);
            stream.seekp(0);
        }
    };
}