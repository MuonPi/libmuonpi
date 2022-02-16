#ifndef MUONPI_SOURCE_BASE_H
#define MUONPI_SOURCE_BASE_H

#include "muonpi/global.h"
#include "muonpi/sink/base.h"
#include "muonpi/threadrunner.h"

#include <atomic>
#include <future>
#include <memory>
#include <queue>

namespace muonpi::source {

template <typename T>
/**
 * @brief The base class
 * Represents a canonical source for items of type T.
 */
class LIBMUONPI_PUBLIC base {
public:
    explicit base(sink::base<T>& sink);

    /**
     * @brief ~base The destructor. If this gets called while the event loop is still running, it
     * will tell the loop to finish and wait for it to be done.
     */
    virtual ~base();

protected:
    /**
     * @brief put pushes an item into the source
     * @param item The item to push
     */
    void put(T item);

private:
    sink::base<T>& m_sink;
};

template <typename T>
base<T>::base(sink::base<T>& sink)
    : m_sink {sink} {}

template <typename T>
base<T>::~base() = default;

template <typename T>
void base<T>::put(T item) {
    m_sink.get(std::move(item));
}

} // namespace muonpi::source

#endif // MUONPI_SOURCE_BASE_H
