#pragma once

#include <queue>              // For std::queue
#include <mutex>              // For std::mutex, std::lock_guard, std::unique_lock
#include <condition_variable> // For std::condition_variable
#include <utility>            // For std::move
#include <string>             // Required for the RTTYmsgOutQ template argument
#include <vector>             // Required if you use vector<char> as the template argument

/**
 * @class ThreadSafeMessageQueue
 * @brief Provides a thread-safe implementation of the Producer-Consumer pattern.
 * All implementation is in the header to avoid linker errors (template rule).
 */
template <typename T>
class ThreadSafeMessageQueue {
public:
    ~ThreadSafeMessageQueue() = default;

    /**
     * @brief Pushes a message onto the back of the queue.
     */
    void push(const T& message) {
        {
            // Lock must be acquired before touching shared data (queue_)
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(message);
        }
        // Notification must be outside the lock scope
        cv_.notify_one();
    }

    /**
     * @brief Blocks the calling thread until a message is available, then returns it.
     */
    T waitAndPop() {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait: Blocks until queue is non-empty
        cv_.wait(lock, [this] { return !queue_.empty(); });

        // Retrieve the message using std::move for efficiency
        T message = std::move(queue_.front());
        queue_.pop();
        return message;
    }

    /**
     * @brief Checks if the queue is currently empty.
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Retrieves the current number of messages in the queue.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    // **CRITICAL:** Queue holds the generic template type T
    std::queue<T> queue_;

    // Mutex to protect concurrent access
    mutable std::mutex mutex_;

    // Condition variable to signal consumer threads
    std::condition_variable cv_;
};
