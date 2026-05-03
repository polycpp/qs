#pragma once

/**
 * @file qs/detail/stringify.hpp
 * @brief Implementation of query string stringification (qs::stringify).
 * @since 0.1.0
 */

#include <string>
#include <vector>
#include <unordered_set>

#include <polycpp/core/json.hpp>
#include <polycpp/core/error.hpp>
#include <polycpp/qs/qs.hpp>
#include <polycpp/qs/detail/utils.hpp>

namespace polycpp {
namespace qs {
namespace detail {

/**
 * @brief Encode a key or value component for the query string.
 * @param str The string to encode.
 * @param opts The stringify options.
 * @param isKey Whether this is a key (controls encodeValuesOnly behavior).
 * @return The encoded string.
 * @since 0.1.0
 */
inline std::string encodeComponent(const std::string& str,
                                    const StringifyOptions& opts,
                                    bool isKey) {
    if (!opts.encode) return str;
    if (opts.encodeValuesOnly && isKey) return str;
    if (opts.charset == "iso-8859-1") {
        return detail::encode(detail::utf8ToLatin1(str), opts.format);
    }
    return detail::encode(str, opts.format);
}

/**
 * @brief Generate the array prefix for a key based on array format.
 * @param prefix The parent key prefix.
 * @param key The array index or empty string.
 * @param format The array format.
 * @return The formatted prefix.
 * @since 0.1.0
 */
inline std::string generateArrayPrefix(const std::string& prefix,
                                        const std::string& key,
                                        ArrayFormat format) {
    switch (format) {
        case ArrayFormat::brackets:
            return prefix + "[]";
        case ArrayFormat::indices:
            return prefix + "[" + key + "]";
        case ArrayFormat::repeat:
            return prefix;
        case ArrayFormat::comma:
            // Comma format is handled specially in the stringify loop
            return prefix;
    }
    return prefix + "[" + key + "]"; // fallback to indices
}

/**
 * @brief Recursive stringify implementation.
 *
 * Walks the object/array tree and produces key=value pairs.
 *
 * @param obj The value to stringify.
 * @param prefix The key prefix for nested values.
 * @param opts The stringify options.
 * @param visited Set of visited object pointers for cycle detection.
 * @param results Output vector of key=value strings.
 * @since 0.1.0
 */
inline void stringifyRecursive(
    const JsonValue& obj,
    const std::string& prefix,
    const StringifyOptions& opts,
    std::unordered_set<const JsonValue*>& visited,
    std::vector<std::string>& results)
{
    // Null handling
    if (obj.isNull()) {
        if (opts.skipNulls) return;
        if (opts.strictNullHandling) {
            results.push_back(encodeComponent(prefix, opts, true));
            return;
        }
        results.push_back(encodeComponent(prefix, opts, true) + "=");
        return;
    }

    // Boolean handling
    if (obj.isBool()) {
        std::string val = obj.asBool() ? "true" : "false";
        results.push_back(
            encodeComponent(prefix, opts, true) + "=" +
            encodeComponent(val, opts, false));
        return;
    }

    // Number handling
    if (obj.isNumber()) {
        double num = obj.asNumber();
        std::string val;
        if (Number::isNaN(num) || !Number::isFinite(num)) {
            val = "";
        } else {
            // Use Number::toString for JS-compatible formatting
            val = Number::toString(num);
        }
        results.push_back(
            encodeComponent(prefix, opts, true) + "=" +
            encodeComponent(val, opts, false));
        return;
    }

    // String handling (leaf)
    if (obj.isString()) {
        results.push_back(
            encodeComponent(prefix, opts, true) + "=" +
            encodeComponent(obj.asString(), opts, false));
        return;
    }

    // Array handling
    if (obj.isArray()) {
        const auto& arr = obj.asArray();

        // Empty array handling
        if (arr.empty()) {
            if (opts.allowEmptyArrays) {
                // Encode the prefix (key) but always emit literal brackets []
                results.push_back(encodeComponent(prefix, opts, true) + "[]");
            }
            return;
        }

        // Comma format: join all elements with comma
        if (opts.arrayFormat == ArrayFormat::comma) {
            // For single-element arrays with commaRoundTrip, use bracket notation
            if (arr.size() == 1 && opts.commaRoundTrip) {
                // Emit as bracket notation for round-trip safety
                stringifyRecursive(arr[0], prefix + "[]", opts, visited, results);
                return;
            }

            // Join all array elements with comma (raw, unencoded)
            std::string joined;
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) joined += ",";
                if (arr[i].isString()) {
                    if (opts.encodeValuesOnly && opts.encode) {
                        // With encodeValuesOnly, encode each element individually
                        joined += detail::encode(arr[i].asString(), opts.format);
                    } else {
                        joined += arr[i].asString();
                    }
                } else if (arr[i].isNumber()) {
                    double num = arr[i].asNumber();
                    if (!Number::isNaN(num) && Number::isFinite(num)) {
                        joined += Number::toString(num);
                    }
                } else if (arr[i].isBool()) {
                    joined += arr[i].asBool() ? "true" : "false";
                } else if (arr[i].isNull()) {
                    // null in comma-joined array is empty string
                }
                // Objects/arrays in comma format are not recursed
            }
            if (!joined.empty()) {
                // With default encode (not encodeValuesOnly), encode the whole
                // joined string including commas
                std::string encodedValue;
                if (opts.encode && !opts.encodeValuesOnly) {
                    encodedValue = detail::encode(joined, opts.format);
                } else {
                    encodedValue = joined;
                }
                results.push_back(
                    encodeComponent(prefix, opts, true) + "=" + encodedValue);
            }
            return;
        }

        // Standard array formats (indices, brackets, repeat)
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& elem = arr[i];
            if (opts.skipNulls && elem.isNull()) continue;

            std::string arrayPrefix = generateArrayPrefix(
                prefix, std::to_string(i), opts.arrayFormat);

            stringifyRecursive(elem, arrayPrefix, opts, visited, results);
        }
        return;
    }

    // Object handling
    if (obj.isObject()) {
        // Cycle detection
        if (visited.count(&obj)) {
            throw RangeError("Cyclic object value");
        }
        visited.insert(&obj);

        const auto& map = obj.asObject();
        for (const auto& [key, val] : map) {
            if (opts.skipNulls && val.isNull()) continue;

            // Build the prefix with raw (unencoded) keys.
            // Encoding happens once at the leaf level via encodeComponent.
            // This matches npm qs behavior and avoids double-encoding.
            std::string childPrefix;
            if (prefix.empty()) {
                childPrefix = key;
            } else if (opts.allowDots && opts.encodeDotInKeys) {
                // Replace in-key dots with sentinel \x01; post-processed
                // in stringifyImpl after leaf-level encoding.
                std::string dotMarked;
                dotMarked.reserve(key.size());
                for (char c : key) {
                    dotMarked += (c == '.') ? '\x01' : c;
                }
                childPrefix = prefix + "." + dotMarked;
            } else if (opts.allowDots) {
                childPrefix = prefix + "." + key;
            } else {
                childPrefix = prefix + "[" + key + "]";
            }

            stringifyRecursive(val, childPrefix, opts, visited, results);
        }

        visited.erase(&obj);
        return;
    }
}

/**
 * @brief Full stringify implementation.
 * @since 0.1.0
 */
inline std::string stringifyImpl(const JsonValue& obj, const StringifyOptions& opts) {
    // Non-object input -> empty string
    if (!obj.isObject()) {
        return "";
    }

    const auto& map = obj.asObject();
    if (map.empty()) {
        return "";
    }

    std::vector<std::string> parts;
    std::unordered_set<const JsonValue*> visited;

    for (const auto& [key, val] : map) {
        if (opts.skipNulls && val.isNull()) continue;

        // Pass raw key as prefix; encoding happens at the leaf level.
        stringifyRecursive(val, key, opts, visited, parts);
    }

    // Post-process encodeDotInKeys sentinel: \x01 marks in-key dots.
    // After leaf-level encoding, \x01 becomes %01 when keys are encoded.
    // Upstream emits %252E in that mode so parse can decode it once to
    // in-key %2E text before decodeDotInKeys restores the dot.
    // Replace only in the key portion to avoid corrupting values.
    if (opts.encodeDotInKeys) {
        const bool encodedKeys = opts.encode && !opts.encodeValuesOnly;
        for (auto& part : parts) {
            auto eqPos = part.find('=');
            size_t keyEnd = (eqPos != std::string::npos) ? eqPos : part.size();
            std::string fixed;
            fixed.reserve(part.size());
            for (size_t i = 0; i < keyEnd; ++i) {
                if (part[i] == '\x01') {
                    fixed += "%2E";
                } else if (i + 2 < keyEnd &&
                           part[i] == '%' && part[i+1] == '0' &&
                           part[i+2] == '1') {
                    fixed += encodedKeys ? "%252E" : "%2E";
                    i += 2;
                } else {
                    fixed += part[i];
                }
            }
            // Append the value portion unchanged
            fixed += part.substr(keyEnd);
            part = std::move(fixed);
        }
    }

    std::string joined;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) joined += opts.delimiter;
        joined += parts[i];
    }

    if (opts.addQueryPrefix && !joined.empty()) {
        joined = "?" + joined;
    }

    return joined;
}

} // namespace detail
} // namespace qs
} // namespace polycpp
