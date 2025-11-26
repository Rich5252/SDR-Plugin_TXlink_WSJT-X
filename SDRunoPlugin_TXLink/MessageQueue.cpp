#include <queue>
#include <mutex>
#include <condition_variable>
#include <string> // Or a struct for complex messages
#include <vector>             // For std::vector

#include "MessageQueue.h"


template <typename T> void ThreadSafeMessageQueue<T>::push(const T& message) {
    {
        // std::lock_guard automatically releases the lock upon exiting the scope
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(message);
    }
    // The notification MUST occur outside the lock to avoid temporary deadlocks
    cv_.notify_one();
}

template <typename T> T ThreadSafeMessageQueue<T>::waitAndPop() {
    // 1. Acquire a unique lock
    std::unique_lock<std::mutex> lock(mutex_);

    // 2. Wait: Blocks the thread until cv_.notify_one() is called AND the queue is NOT empty
    cv_.wait(lock, [this] { return !queue_.empty(); });

    // 3. Extract and remove the message
    T message = std::move(queue_.front());
    queue_.pop();

    return message;
}

// Implementation for size()
template <typename T> size_t ThreadSafeMessageQueue<T>::size() const {
    // Lock the mutex to ensure the read of queue_.size() is thread-safe
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

// Implementation for empty()
template <typename T> bool ThreadSafeMessageQueue<T>::empty() const {
    // Lock the mutex to ensure the read of queue_.empty() is thread-safe
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}
