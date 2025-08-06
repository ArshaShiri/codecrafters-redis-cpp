#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std::chrono_literals;


template<typename Key, typename Value>
class DataManager {
    static inline constexpr auto MAX_DURATION = std::chrono::milliseconds::max();

    struct ValueWithExpiry {
        Value value;
        std::chrono::steady_clock::time_point time_added{std::chrono::steady_clock::now()};
        std::chrono::milliseconds duration{MAX_DURATION};
    };

  public:
    /**
     * @brief Sets a key-value pair with an optional expiry duration.
     *
     * @param key The key to set.
     * @param value The value to associate with the key.
     * @param duration Optional duration after which the key will expire. Default is 0 (no expiry).
     * @return true if the key was set successfully, false if the key already exists.
     */
    bool set(Key key, Value value, std::chrono::milliseconds duration = MAX_DURATION) {
        ValueWithExpiry new_value;
        new_value.value = std::move(value);

        std::cout << "Setting key: " << key << " with value: " << new_value.value << std::endl
                  << "Duration: " << duration.count() << "ms" << std::endl;

        if (duration != MAX_DURATION) {
            new_value.duration = duration;
        }

        const auto result = data_store_.emplace(std::move(key), std::move(new_value));
        return result.second;
    }

    /**
     * @brief Gets the value associated with a given key.
     *
     * @param key The key to look up.
     * @return A pair where the first element is true if the key exists, and the second element is the value or a
     * default-constructed Value if the key does not exist or has expired.
     */
    std::pair<bool, Value> get(const Key &key) {
        const auto it = data_store_.find(key);
        if (it == data_store_.end()) {
            return {false, Value()};
        }

        const auto &value_with_expiry = it->second;

        if (value_with_expiry.duration != MAX_DURATION) {
            std::cout << "has expiry" << std::endl;
            const auto now = std::chrono::steady_clock::now();
            const auto time_diff =
              std::chrono::duration_cast<std::chrono::milliseconds>(now - value_with_expiry.time_added);
            std::cout << "time_diff: " << time_diff.count() << std::endl;
            std::cout << "value_with_expiry.duration: " << value_with_expiry.duration.count() << std::endl;

            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            std::cout << "now: " << ms << std::endl;

            if (time_diff >= value_with_expiry.duration) {
                // Key has expired, remove it
                data_store_.erase(it);
                return {false, Value()};
            }
        }

        return {true, it->second.value};
    }

  private:
    std::unordered_map<Key, ValueWithExpiry> data_store_;
};
