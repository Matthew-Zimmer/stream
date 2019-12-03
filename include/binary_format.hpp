#pragma once
#include <algorithm>
#include <cstring>
#include <fstream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include <reflection/reflection.hpp>

#include "serializable.hpp"

namespace Slate::Stream
{
    namespace Binary_Formatting
    {
        class ADL_BF {};

        template <typename Type>
        auto Size(Type const& type, ADL_BF bf) -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, std::size_t>
        {
            return sizeof(type);
        }

        std::size_t Size(std::string const& string, ADL_BF bf)
        {
            return string.size() + sizeof(std::size_t);
        }

        template <typename Type>
        auto Size(std::vector<Type> const& vector, ADL_BF bf) -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, std::size_t>
        {
            return vector.size() ? vector.size() * Size(vector[0], bf) : 0;
        }

        template <typename Type>
        auto Size(std::vector<Type> const& vector, ADL_BF bf) -> std::enable_if_t<!std::is_trivially_copyable_v<std::remove_reference_t<Type>>, std::size_t>
        {
            return std::accumulate(vector.begin(), vector.end(), static_cast<size_t>(0), [bf](auto& x, auto& y){ return x + Size(y, bf); });
        }

        template <typename ... Types>
        std::size_t Size(std::tuple<Types...> const& tuple, ADL_BF bf)
        {
            return std::apply([bf](auto const& ... args){ return (Size(args, bf) + ...); }, tuple);
        }

        template <typename Key, typename Value>
        auto Size(std::unordered_map<Key, Value> const& umap, ADL_BF bf) -> std::enable_if_t<std::is_scalar_v<Key> && std::is_scalar_v<Value>, std::size_t>
        {
            return umap.size() ? umap.size() * (Size(umap.begin()->first, bf) + Size(umap.begin()->second, bf)) : 0;
        }
        
        template <typename Key, typename Value>
        auto Size(std::unordered_map<Key, Value> const& umap, ADL_BF bf) -> std::enable_if_t<!std::is_scalar_v<Key> && std::is_scalar_v<Value>, std::size_t>
        {
            return umap.size() ? umap.size() * Size(umap.begin()->second, bf) + std::accumulate(umap.begin(), umap.end(), static_cast<std::size_t>(0), [bf](auto const& x, auto const& y){ return x + Size(y.first, bf); }) : 0;
        }

        template <typename Key, typename Value>
        auto Size(std::unordered_map<Key, Value> const& umap, ADL_BF bf) -> std::enable_if_t<std::is_scalar_v<Key> && !std::is_scalar_v<Value>, std::size_t>
        {
            return umap.size() ? umap.size() * Size(umap.begin()->first, bf) + std::accumulate(umap.begin(), umap.end(), static_cast<std::size_t>(0), [bf](auto const& x, auto const& y){ return x + Size(y.second, bf); }) : 0;
        }

        template <typename Key, typename Value>
        auto Size(std::unordered_map<Key, Value> const& umap, ADL_BF bf) -> std::enable_if_t<!std::is_scalar_v<Key> && !std::is_scalar_v<Value>, std::size_t>
        {
            return std::accumulate(umap.begin(), umap.end(), static_cast<std::size_t>(0), [bf](auto const& x, auto const& y){ return x + Size(y.first, bf); }) + std::accumulate(umap.begin(), umap.end(), static_cast<std::size_t>(0), [](auto const& x, auto const& y){ return x + Size(y.second); });
        }

        template <typename Type, typename ... Types>
        auto Size(Serializable<Type, Types...> const& s, ADL_BF bf)
        {
            std::size_t sum{ 0 };
            ([&](){
                if constexpr(Is_Variable<Types>)
                    sum += Size(Meta::Cast<Types, Type>(s).Variable(), bf);
            }(), ...);
            return sum;
        }
        

        template <typename Type>
        auto To(Type const& any, std::vector<char>::iterator iter, ADL_BF bf) -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, std::vector<char>::iterator>
        {
            auto size = Size(any, bf);
            std::memmove(&*iter, reinterpret_cast<void const*>(&any), size);
            return iter + size;
        }
        
        auto To(std::string const& string, std::vector<char>::iterator iter, ADL_BF bf)
        {
            iter = To(string.size(), iter, bf);
            return std::copy(string.begin(), string.end(), iter);
        }

        template <typename Type>
        auto To(std::vector<Type> const& vector, std::vector<char>::iterator iter, ADL_BF bf)
        {
            iter = To(vector.size(), iter, bf);
            for (auto& x : vector)
                iter = To(x, iter, bf);
            return iter;
        }
        
        template <typename ... Types>
        auto To(std::tuple<Types...> const& tuple, std::vector<char>::iterator iter, ADL_BF bf)
        {
            return std::apply([&](auto const& ... args){ return ((iter = To(args, iter, bf)), ...); }, tuple);
        }

        template <typename Key, typename Value>
        auto To(std::unordered_map<Key, Value> const& umap, std::vector<char>::iterator iter, ADL_BF bf)
        {
            iter = To(umap.size(), iter, bf);
            for (auto& [k, v] : umap)
            {
                iter = To(k, iter, bf);
                iter = To(v, iter, bf);
            }
            return iter;
        }

        template <typename Type, typename ... Types>
        auto To(Serializable<Type, Types...> const& s, std::vector<char>::iterator iter, ADL_BF bf)
        {
            ([&](){
                if constexpr(Is_Variable<Types>)
                    iter = To(Meta::Cast<Types, Type>(s).Variable(), iter, bf);
            }(), ...);
            return iter;
        }

        template <typename Type>
        auto From(Type& any, std::vector<char>::iterator iter, ADL_BF bf) -> std::enable_if_t<std::is_trivially_copyable_v<std::remove_reference_t<Type>>, std::vector<char>::iterator>
        { 
            auto size = Size(any, bf);
            std::memmove(reinterpret_cast<void*>(&any), &*iter, size);
            return iter + size;
        }

        auto From(std::string& string, std::vector<char>::iterator iter, ADL_BF bf)
        { 
            std::size_t size{ 0 };
            iter = From(size, iter, bf);
            string.resize(size);
            auto end = iter + size;
            std::copy(iter, end, string.begin());
            return end;
        }
        
        template <typename Type>
        auto From(std::vector<Type>& vector, std::vector<char>::iterator iter, ADL_BF bf)
        { 
            std::size_t size{ 0 };
            iter = From(size, iter, bf);
            vector.resize(size);
            for (std::size_t i{0};i < size;i++)
                iter = From(vector[i], iter, bf);
            return iter;
        }

        template <typename ... Types>
        auto From(std::tuple<Types...>& tuple, std::vector<char>::iterator iter, ADL_BF bf)
        { 
            return std::apply([&](auto& ... args){ return ((iter = From(args, iter, bf)), ...); }, tuple);
        }

        template <typename Key, typename Value>
        auto From(std::unordered_map<Key, Value>& umap, std::vector<char>::iterator iter, ADL_BF bf)
        { 
            std::size_t size{ 0 };
            iter = From(size, iter, bf);
            umap.resize(size);
            Key k;
            for (std::size_t i{0}; i < size; i++)
            {
                iter = From(k, iter, bf);
                iter = From(umap[k], iter, bf);
            }
            return iter;
        }
        
        template <typename Type, typename ... Types>
        auto From(Serializable<Type, Types...>& s, std::vector<char>::iterator iter, ADL_BF bf)
        {
            ([&](){
                if constexpr(Is_Variable<Types>)
                    iter = From(Meta::Cast<Types, Type>(s).Variable(), iter, bf);
            }(), ...);
            return iter;
        }
    }

    /*
        Summary:
            Adds the binary format functions to a stream type
    */
    template <typename Stream>
    class Binary_Format
    {
    public:
        template <typename Type>
        auto Size(Type const& t)
        {
            return Binary_Formatting::Size(t, Binary_Formatting::ADL_BF{});
        }

        template <typename Type>
        auto To(Type const& t, std::vector<char>::iterator iter)
        {
            return Binary_Formatting::To(t, iter, Binary_Formatting::ADL_BF{});
        }
        
        template <typename Type>
        auto From(Type& t, std::vector<char>::iterator iter)
        {
            return Binary_Formatting::From(t, iter, Binary_Formatting::ADL_BF{});
        }

        template <typename Type>
        auto Output(Type const& t)
        {
            class O
            {
                Stream& me;
                Type const& t;
            public:
                O(Stream& me, Type const& t) : me{ me }, t{ t } {}
                Stream& Write_Bytes()
                {
                    std::tuple tup{ me.Size(t), t };
                    std::vector<char> bytes(sizeof(std::size_t) + std::get<0>(tup));
                    me.To(tup, bytes.begin());
                    me.Write(bytes);
                    return me;
                }
            } x { Meta::Cast<Stream>(*this), t };
            return x;
        }

        template <typename Type>
        auto Input(Type& t)
        {
            class I
            {
                Stream& me;
                Type& t;
            public:
                I(Stream& me, Type& t): me{ me }, t{ t } {}
                Stream& Read_Bytes()
                {
                    auto size = me.template Read<std::size_t>();
                    auto bytes = me.Read(size); 
                    me.From(t, bytes.begin());
                    return me;
                }
            } x { Meta::Cast<Stream>(*this), t };
            return x;
        }
    };
}