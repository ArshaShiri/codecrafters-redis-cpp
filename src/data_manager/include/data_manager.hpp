#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

template<typename Key, typename Value>
class DataManager {
  public:
    /**
     * @brief Sets the value for a given key.
     *
     * @param key The key to set.
     * @param value The value to associate with the key.
     * @return true if the key-value pair was added, false if the key already exists.
     */
    bool set(Key key, Value value) {
        std::cout << "Setting value for key: " << key << " to " << value << std::endl;
        const auto result = data_store_.emplace(std::move(key), std::move(value));
        return result.second;
    }

    /**
     * @brief Gets the value associated with a given key.
     *
     * @param key The key to look up.
     * @return A pair where the first element is true if the key exists, and the second element is the value or a
     * default-constructed Value if the key does not exist.
     */
    std::pair<bool, Value> get(const Key &key) const {
        std::cout << "Getting value for key: " << key << std::endl;
        const auto it = data_store_.find(key);
        if (it == data_store_.end()) {
            std::cout << "Key not found: " << key << std::endl;
            return {false, Value()};
        }

        return {true, it->second};
    }

  private:
    std::unordered_map<Key, Value> data_store_;
};
