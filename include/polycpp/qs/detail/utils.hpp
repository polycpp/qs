#pragma once

/**
 * @file qs/detail/utils.hpp
 * @brief Internal utility functions for percent-encoding/decoding and merging.
 * @since 0.1.0
 */

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <polycpp/core/json.hpp>
#include <polycpp/qs/qs.hpp>

namespace polycpp {
namespace qs {
namespace detail {

/// @brief Characters that should NOT be percent-encoded (RFC3986 unreserved set).
inline bool isUnreserved(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '-' || c == '_' || c == '.' || c == '~';
}

/// @brief Convert a hex character to its numeric value.
/// @return The value 0-15, or -1 if not a hex digit.
inline int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/**
 * @brief Percent-encode a string per RFC3986 (UTF-8).
 *
 * Encodes all characters except the RFC3986 unreserved set:
 * `A-Z a-z 0-9 - _ . ~`
 *
 * @param str The input string to encode.
 * @param format The URI format (RFC3986 uses %20 for spaces, RFC1738 uses +).
 * @return The percent-encoded string.
 * @since 0.1.0
 */
inline std::string encode(const std::string& str, Format format = Format::RFC3986) {
    static const char hexChars[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(str.size() * 3); // worst case

    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (isUnreserved(static_cast<char>(c))) {
            result += static_cast<char>(c);
        } else if (c == ' ' && format == Format::RFC1738) {
            result += '+';
        } else {
            result += '%';
            result += hexChars[(c >> 4) & 0x0F];
            result += hexChars[c & 0x0F];
        }
    }
    return result;
}

/**
 * @brief Percent-decode a string.
 *
 * Decodes `%XX` sequences and replaces `+` with space (as per
 * application/x-www-form-urlencoded).
 *
 * @param str The percent-encoded string.
 * @return The decoded string.
 * @since 0.1.0
 */
inline std::string decode(const std::string& str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int hi = hexVal(str[i + 1]);
            int lo = hexVal(str[i + 2]);
            if (hi >= 0 && lo >= 0) {
                result += static_cast<char>((hi << 4) | lo);
                i += 2;
                continue;
            }
        }
        if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

/**
 * @brief Split a string by a single-character or multi-character delimiter.
 * @param str The string to split.
 * @param delim The delimiter string.
 * @param limit Maximum number of parts (0 = unlimited).
 * @return Vector of split parts.
 * @since 0.1.0
 */
inline std::vector<std::string> split(const std::string& str,
                                       const std::string& delim,
                                       int limit = 0) {
    std::vector<std::string> parts;
    if (str.empty()) return parts;
    if (delim.empty()) {
        parts.push_back(str);
        return parts;
    }

    size_t start = 0;
    size_t pos;
    while ((pos = str.find(delim, start)) != std::string::npos) {
        parts.push_back(str.substr(start, pos - start));
        start = pos + delim.size();
        // JS-style split: limit means max number of elements returned
        if (limit > 0 && static_cast<int>(parts.size()) >= limit) {
            return parts;
        }
    }
    parts.push_back(str.substr(start));
    // Apply limit if we have more parts than allowed
    if (limit > 0 && static_cast<int>(parts.size()) > limit) {
        parts.resize(limit);
    }
    return parts;
}

/**
 * @brief Deep-merge source into target, combining arrays/objects recursively.
 *
 * Implements the merge algorithm from npm qs's utils.merge():
 * - Array + Array: element-wise merge
 * - Object + Object: key-wise merge
 * - Two values of incompatible types: wrap in array
 *
 * @param target The target value.
 * @param source The source value to merge in.
 * @return The merged result.
 * @since 0.1.0
 */
inline JsonValue merge(const JsonValue& target, const JsonValue& source) {
    // If source is null, return target as-is
    if (source.isNull()) return target;

    // If target is null, return source
    if (target.isNull()) return source;

    // Both are objects: key-wise merge
    if (target.isObject() && source.isObject()) {
        JsonObject result = target.asObject();
        for (const auto& [key, val] : source.asObject()) {
            auto it = result.find(key);
            if (it != result.end()) {
                it->second = merge(it->second, val);
            } else {
                result[key] = val;
            }
        }
        return JsonValue(std::move(result));
    }

    // Both are arrays: element-wise merge
    if (target.isArray() && source.isArray()) {
        JsonArray result = target.asArray();
        const auto& srcArr = source.asArray();
        for (size_t i = 0; i < srcArr.size(); ++i) {
            if (i < result.size()) {
                if (result[i].isObject() && srcArr[i].isObject()) {
                    result[i] = merge(result[i], srcArr[i]);
                } else if (!srcArr[i].isNull()) {
                    // If the target slot is null/undefined, just replace
                    if (result[i].isNull()) {
                        result[i] = srcArr[i];
                    } else {
                        // Both are non-null primitives at same index: keep target
                        // (qs behavior: first value wins in array merge)
                    }
                }
            } else {
                result.push_back(srcArr[i]);
            }
        }
        return JsonValue(std::move(result));
    }

    // Array target + object source: convert array to object, then merge
    if (target.isArray() && source.isObject()) {
        JsonObject result;
        const auto& arr = target.asArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            if (!arr[i].isNull()) {
                result[std::to_string(i)] = arr[i];
            }
        }
        for (const auto& [key, val] : source.asObject()) {
            auto it = result.find(key);
            if (it != result.end()) {
                it->second = merge(it->second, val);
            } else {
                result[key] = val;
            }
        }
        return JsonValue(std::move(result));
    }

    // Object target + array source: convert array to object, then merge
    if (target.isObject() && source.isArray()) {
        JsonObject result = target.asObject();
        const auto& arr = source.asArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            std::string key = std::to_string(i);
            auto it = result.find(key);
            if (it != result.end()) {
                it->second = merge(it->second, arr[i]);
            } else {
                if (!arr[i].isNull()) {
                    result[key] = arr[i];
                }
            }
        }
        return JsonValue(std::move(result));
    }

    // Array target + primitive source: append to array
    if (target.isArray()) {
        JsonArray result = target.asArray();
        result.push_back(source);
        return JsonValue(std::move(result));
    }

    // Primitive target + array source: prepend target to source array
    if (source.isArray()) {
        JsonArray result;
        result.push_back(target);
        const auto& srcArr = source.asArray();
        for (const auto& elem : srcArr) {
            result.push_back(elem);
        }
        return JsonValue(std::move(result));
    }

    // Two primitives or incompatible: wrap both in an array
    JsonArray result;
    result.push_back(target);
    result.push_back(source);
    return JsonValue(std::move(result));
}

/**
 * @brief Compact sparse arrays by removing null/undefined holes.
 *
 * Uses BFS traversal to find all arrays in the value tree, then
 * removes null entries from each array (deepest first).
 *
 * @param value The value to compact.
 * @return The compacted value.
 * @since 0.1.0
 */
inline JsonValue compact(const JsonValue& value) {
    if (!value.isObject() && !value.isArray()) {
        return value;
    }

    // Work on a mutable copy
    JsonValue result = value;

    // BFS queue: collect pointers to all nested arrays/objects
    struct QueueItem {
        JsonValue* parent;
        std::string key;
        size_t index;
        bool isArrayParent;
    };

    std::vector<JsonValue*> toCompact;

    // Recursive helper to compact arrays in-place
    std::function<void(JsonValue&)> compactRecursive;
    compactRecursive = [&](JsonValue& val) {
        if (val.isArray()) {
            // First, recurse into children
            auto& arr = val.asArray();
            for (auto& elem : arr) {
                compactRecursive(elem);
            }
            // Then compact: remove null entries
            JsonArray compacted;
            for (auto& elem : arr) {
                if (!elem.isNull()) {
                    compacted.push_back(std::move(elem));
                }
            }
            arr = std::move(compacted);
        } else if (val.isObject()) {
            auto& obj = val.asObject();
            for (auto& [k, v] : obj) {
                compactRecursive(v);
            }
        }
    };

    compactRecursive(result);
    return result;
}

/**
 * @brief Combine two values for duplicate key handling.
 *
 * If target is already an array, append source. Otherwise create
 * a new array containing both values.
 *
 * @param target Existing value.
 * @param source New value to combine.
 * @return Combined value (always an array).
 * @since 0.1.0
 */
inline JsonValue combine(const JsonValue& target, const JsonValue& source) {
    if (target.isArray()) {
        JsonArray result = target.asArray();
        result.push_back(source);
        return JsonValue(std::move(result));
    }
    JsonArray result;
    result.push_back(target);
    result.push_back(source);
    return JsonValue(std::move(result));
}

} // namespace detail
} // namespace qs
} // namespace polycpp
