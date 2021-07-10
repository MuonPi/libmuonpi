#ifndef BASE64_H
#define BASE64_H

#include "muonpi/global.h"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace muonpi::base64 {

/**
 * @brief decode Decode a base64 encoded string
 * @param val
 * @return
 */
[[nodiscard]] inline auto LIBMUONPI_PUBLIC decode(std::string_view val) -> std::string
{
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
    });
}

/**
 * @brief encode Encode a string to base64
 * @param val
 * @return
 */
[[nodiscard]] inline auto LIBMUONPI_PUBLIC encode(std::string_view val) -> std::string
{
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

}

#endif // BASE64_H
