#pragma once
#include <string>

#include <reflection/reflection.hpp>

namespace Slate
{
    namespace Stream
    {
        //General Stream class
        template <template <typename> typename ... FTypes>
        class Stream : public Is<Stream<FTypes...>, Features<FTypes...>>
        {
        public:
            Stream(std::string const& filename)
            {
                this->Open(filename);
            }
            Stream() = default;

            template <typename Type>
            friend Stream& operator<<(Stream& s, Type const& t)
            {
                return s.Output(t).Write_Bytes();
            }
            
            template <typename Type>
            friend Stream& operator>>(Stream& s, Type& o)
            {
                return s.Input(o).Read_Bytes();
            }
        };
    }
}