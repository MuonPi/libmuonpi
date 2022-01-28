#ifndef MUONPI_TYPES_H
#define MUONPI_TYPES_H

#include <vector>

namespace muonpi {

template <typename T, typename OutputV, template <typename V = OutputV> class OutputIt,
    std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true,
    std::enable_if_t<std::is_base_of<std::insert_iterator<OutputV>, OutputIt<OutputV>>::value, bool> = true>
void to_bytes(const T& value, OutputIt<OutputV> bytes)
{
    const auto* begin = reinterpret_cast<const std::byte*>(std::addressof(value));
    const auto* end = begin + sizeof(T);
    std::copy(begin, end, bytes);
}

template <typename T,
    std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true>
[[nodiscard]] auto from_bytes(std::vector<std::byte>::iterator bytes) -> T
{
    T value {};

    auto* begin_value = reinterpret_cast<std::byte*>(std::addressof(value));
    std::copy(bytes, bytes + sizeof(T), begin_value);

    return value;
}

} // namespace muonpi

#endif // MUONPI_TYPES_H
