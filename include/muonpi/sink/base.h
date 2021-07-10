#ifndef SINKBASE_H
#define SINKBASE_H

#include "muonpi/global.h"

#include "muonpi/threadrunner.h"
#include <array>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace muonpi::sink {

template <typename T>
/**
 * @brief The sink class
 * Represents a canonical sink for items of type T.
 */
class LIBMUONPI_PUBLIC base {
public:
    /**
     * @brief ~sink The destructor. If this gets called while the event loop is still running, it will tell the loop to finish and wait for it to be done.
     */
    virtual ~base();

    /**
     * @brief get pushes an item into the sink
     * @param item The item to push
     */
    virtual void get(T item) = 0;
};

template <typename T>
class LIBMUONPI_PUBLIC threaded : public base<T>, public thread_runner {
public:
    /**
     * @brief threaded
     * @param name The name of the thread. Useful for identification.
     * @param timeout The timeout for how long the thread should wait before calling the process method without parameter.
     */
    threaded(const std::string& name, std::chrono::milliseconds timeout);

    /**
     * @brief threaded
     * @param name The name of the thread. Useful for identification.
     */
    threaded(const std::string& name);

    virtual ~threaded() override;

protected:
    /**
     * @brief internal_get
     * @param item The item that is available
     */
    void internal_get(T item);

    /**
     * @brief step Reimplemented from thread_runner.
     * Internally this uses the timeout given in the constructor, default is 5 seconds.
     * It waits for a maximum of timeout, if there is no item available,
     * it calls the process method without parameter, if yes it calls the overloaded process method with the item as parameter.
     * @return the return code of the process methods. If it is nonzero, the Thread will finish.
     */
    [[nodiscard]] auto step() -> int override;

    /**
     * @brief process Gets called whenever a new item is available.
     * @param item The next available item
     * @return The status code which is passed along by the step method.
     */
    [[nodiscard]] virtual auto process(T item) -> int = 0;
    /**
     * @brief process Gets called periodically whenever there is an item available.
     * @return The status code which is passed along by the step method.
     */
    [[nodiscard]] virtual auto process() -> int;

private:
    std::chrono::milliseconds m_timeout { std::chrono::seconds { 5 } };
    std::queue<T> m_items {};
    std::mutex m_mutex {};
    std::condition_variable m_has_items {};
};

template <typename T>
class LIBMUONPI_PUBLIC collection : public threaded<T> {
public:
    /**
     * @brief collection A collection of multiple sinks
     * @param sinks The sinks where the items should be distributed
     */
    collection(std::vector<base<T>*> sinks, const std::string& name = "muon::sink");
    collection(const std::string& name = "muon::sink");

    ~collection() override;

    void get(T item) override;

    void emplace(base<T>& sink);

protected:
    /**
     * @brief get Reimplemented from threaded<T>. gets called when a new item is available
     * @param item The item that is available
     * @return the status code. @see threaded::process
     */
    [[nodiscard]] auto process(T item) -> int override;

private:
    struct forward {
        base<T>& sink;

        inline void put(T item)
        {
            sink.get(std::move(item));
        }
    };

    std::vector<forward> m_sinks {};
};

template <typename T>
base<T>::~base() = default;

template <typename T>
threaded<T>::threaded(const std::string& name)
    : thread_runner { name }
{
    start();
}

template <typename T>
threaded<T>::threaded(const std::string& name, std::chrono::milliseconds timeout)
    : thread_runner { name }
    , m_timeout { timeout }
{
    start();
}

template <typename T>
threaded<T>::~threaded() = default;

template <typename T>
void threaded<T>::internal_get(T item)
{
    std::scoped_lock<std::mutex> lock { m_mutex };
    m_items.push(item);
    m_condition.notify_all();
}

template <typename T>
auto threaded<T>::step() -> int
{
    std::mutex mx;
    std::unique_lock<std::mutex> wait_lock { mx };
    if ((m_items.empty()) && (m_condition.wait_for(wait_lock, m_timeout) == std::cv_status::timeout)) {
        return process();
    }
    if (m_quit) {
        return 0;
    }

    std::size_t n { 0 };
    do {
        auto item = [this]() -> T {
            std::scoped_lock<std::mutex> lock { m_mutex };
            T res = m_items.front();
            m_items.pop();
            return res;
        }();

        int result { process(item) };
        if (result != 0) {
            return result;
        }
        n++;
    } while (!m_items.empty() && (n < 10));
    if (!m_items.empty()) {
        m_condition.notify_all();
    }
    return process();
}

template <typename T>
auto threaded<T>::process() -> int
{
    return 0;
}

template <typename T>
collection<T>::collection(std::vector<base<T>*> sinks, const std::string& name)
    : threaded<T> { name }
    , m_sinks { std::move(sinks) }
{
}

template <typename T>
collection<T>::collection(const std::string& name)
    : threaded<T> { name }
{
}

template <typename T>
collection<T>::~collection() = default;

template <typename T>
void collection<T>::get(T item)
{
    threaded<T>::internal_get(std::move(item));
}

template <typename T>
auto collection<T>::process(T item) -> int
{
    for (auto& fwd : m_sinks) {
        fwd.put(item);
    }
    return 0;
}

template <typename T>
void collection<T>::emplace(base<T>& sink)
{
    m_sinks.emplace_back(forward { sink });
}

}

#endif // SINKBASE_H
