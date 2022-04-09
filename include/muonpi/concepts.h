#ifndef MUONPI_CONCEPTS_H
#define MUONPI_CONCEPTS_H

#include <concepts>
#include <iostream>
#include <iterator>

namespace muonpi {

template <typename T>
concept printable = requires(T t, std::ostream& s) {
    s << t;
};

template <typename T>
concept printable_iterable = requires(T t, std::ostream& s) {
    t.begin();
    t.end();
    { typename std::iterator_traits<decltype(t.begin())>::value_type() }
    ->printable;
};

template <typename T, typename C>
concept iterable = requires(T t) {
    t.begin();
    t.end();
    { typename std::iterator_traits<decltype(t.begin())>::value_type() }
    ->std::same_as<C>;
};

template <typename T, typename S>
concept is_integral_size = std::is_fundamental_v<T>&& std::is_integral<T>::value
                        && (sizeof(T) == sizeof(S)) && (!std::is_class_v<T>);

template <typename T, typename C>
concept iterator = requires(T) {
    { typename std::iterator_traits<T>::value_type() }
    ->std::same_as<C>;
};
} // namespace muonpi

#endif // MUONPI_CONCEPTS_H
