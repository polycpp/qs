/**
 * @file qs.cpp
 * @brief Compiled translation unit for polycpp::qs.
 * @since 0.1.0
 */

#include <polycpp/qs/detail/aggregator.hpp>

polycpp::JsonValue polycpp::qs::parse(const std::string& str, const ParseOptions& opts) {
    return detail::parseImpl(str, opts);
}

std::string polycpp::qs::stringify(const JsonValue& obj, const StringifyOptions& opts) {
    return detail::stringifyImpl(obj, opts);
}
