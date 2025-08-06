#include <charconv>
#include <limits>
#include <stdexcept>

#include "common.hpp"

int get_int_from_string_view(const std::string_view &int_view) {
    int result{};
    const auto [ptr, error_code] = std::from_chars(int_view.data(), int_view.data() + int_view.size(), result);

    if (error_code == std::errc::invalid_argument) {
        return std::numeric_limits<int>::max();
    }

    return result;
}
