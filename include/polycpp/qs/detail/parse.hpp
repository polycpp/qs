#pragma once

/**
 * @file qs/detail/parse.hpp
 * @brief Implementation of query string parsing (qs::parse).
 * @since 0.1.0
 */

#include <string>
#include <vector>
#include <regex>
#include <cmath>

#include <polycpp/core/json.hpp>
#include <polycpp/core/error.hpp>
#include <polycpp/core/number.hpp>
#include <polycpp/qs/qs.hpp>
#include <polycpp/qs/detail/utils.hpp>

namespace polycpp {
namespace qs {
namespace detail {

/**
 * @brief Split a key like "a[b][c]" into segments ["a", "[b]", "[c]"].
 *
 * If allowDots is enabled, dot notation is first converted to bracket notation.
 * Respects the depth limit: segments beyond the limit are concatenated.
 *
 * @param key The flat key from parseValues.
 * @param opts Parse options (depth, allowDots, etc.).
 * @return Vector of key segments.
 * @throws polycpp::RangeError If strictDepth is true and depth is exceeded.
 * @since 0.1.0
 */
inline std::vector<std::string> splitKeyIntoSegments(
    const std::string& key, const ParseOptions& opts)
{
    std::string workKey = key;

    // Convert dot notation to bracket notation if enabled
    if (opts.allowDots || opts.decodeDotInKeys) {
        // If decodeDotInKeys, we need to decode %2E first
        if (opts.decodeDotInKeys) {
            // Replace %2E (case-insensitive) with dots for key parsing
            std::string decoded;
            decoded.reserve(workKey.size());
            for (size_t i = 0; i < workKey.size(); ++i) {
                if (i + 2 < workKey.size() && workKey[i] == '%' &&
                    (workKey[i+1] == '2') &&
                    (workKey[i+2] == 'E' || workKey[i+2] == 'e')) {
                    decoded += '.';
                    i += 2;
                } else {
                    decoded += workKey[i];
                }
            }
            workKey = decoded;
        }

        // Convert dot notation to bracket notation: a.b.c -> a[b][c]
        std::string converted;
        bool first = true;
        size_t i = 0;
        while (i < workKey.size()) {
            if (workKey[i] == '.' && !first && workKey[i] != '[') {
                // Check if we're not inside brackets
                converted += '[';
                ++i;
                // Find the end of the segment (next dot or bracket)
                while (i < workKey.size() && workKey[i] != '.' && workKey[i] != '[') {
                    converted += workKey[i];
                    ++i;
                }
                converted += ']';
            } else if (workKey[i] == '[') {
                first = false;
                converted += workKey[i];
                ++i;
            } else {
                if (workKey[i] == '.') {
                    // Skip the dot and start bracketing
                    first = false;
                    ++i;
                    continue;
                }
                converted += workKey[i];
                ++i;
                if (first) {
                    // Still in the parent segment
                    // Check if next char is a dot or bracket
                    if (i < workKey.size() && (workKey[i] == '.' || workKey[i] == '[')) {
                        first = false;
                    }
                }
            }
        }
        workKey = converted;
    }

    if (opts.depth <= 0) {
        return {workKey};
    }

    // Find the first bracket group
    size_t bracketStart = workKey.find('[');
    if (bracketStart == std::string::npos) {
        return {workKey};
    }

    // Extract parent (everything before the first bracket)
    std::string parent = workKey.substr(0, bracketStart);
    std::vector<std::string> segments;
    if (!parent.empty()) {
        segments.push_back(parent);
    }

    // Extract bracket groups up to depth limit
    size_t pos = bracketStart;
    int depthCount = 0;
    while (pos < workKey.size() && workKey[pos] == '[') {
        size_t closePos = workKey.find(']', pos);
        if (closePos == std::string::npos) {
            // Malformed: no closing bracket, treat rest as literal
            segments.push_back(workKey.substr(pos));
            return segments;
        }

        std::string segment = workKey.substr(pos, closePos - pos + 1);
        depthCount++;

        if (depthCount <= opts.depth) {
            segments.push_back(segment);
        } else {
            // Beyond depth limit
            if (opts.strictDepth) {
                throw RangeError("The depth exceeded the limit of " +
                    std::to_string(opts.depth));
            }
            // Concatenate all remaining brackets as a single bracket key
            // e.g., with depth=1, "a[b][c][d]" -> segments ["a", "[b]"]
            // and remaining "[c][d]" becomes "[c][d]" as a literal key
            std::string remaining = workKey.substr(pos);
            // Wrap in brackets to make it a single segment: "[[c][d]]"
            segments.push_back("[" + remaining + "]");
            return segments;
        }

        pos = closePos + 1;
    }

    return segments;
}

/**
 * @brief Build a nested structure from key segments, from leaf to root.
 *
 * Given segments ["a", "[b]", "[0]", "[c]"] and value "hello",
 * produces { a: { b: [{ c: "hello" }] } }.
 *
 * @param chain The key segments from splitKeyIntoSegments.
 * @param val The value to nest.
 * @param opts Parse options.
 * @return The nested JsonValue structure.
 * @since 0.1.0
 */
inline JsonValue parseObject(
    const std::vector<std::string>& chain,
    const JsonValue& val,
    const ParseOptions& opts)
{
    JsonValue leaf = val;

    for (int i = static_cast<int>(chain.size()) - 1; i >= 0; --i) {
        const std::string& root = chain[i];

        if (root == "[]" && opts.parseArrays) {
            // Empty brackets -> wrap in array (using JS concat semantics)
            if (opts.allowEmptyArrays &&
                (leaf.isNull() || (leaf.isString() && leaf.asString().empty()))) {
                leaf = JsonValue(JsonArray{});
            } else if (leaf.isArray()) {
                // JS [].concat(array) spreads: keep the array as-is
                // (don't wrap array in another array)
            } else {
                JsonArray arr;
                arr.push_back(std::move(leaf));
                leaf = JsonValue(std::move(arr));
            }
        } else {
            JsonObject obj;
            // Strip brackets if present
            std::string cleanRoot = root;
            if (root.size() >= 2 && root.front() == '[' && root.back() == ']') {
                cleanRoot = root.substr(1, root.size() - 2);
            }

            // Check if this is a valid array index
            bool isValidArrayIndex = false;
            int index = -1;
            if (!cleanRoot.empty() && opts.parseArrays) {
                // Use Number::parseInt for JS-compatible parsing
                double parsed = Number::parseInt(cleanRoot, 10);
                if (!Number::isNaN(parsed) &&
                    root != cleanRoot && // Was wrapped in brackets
                    std::to_string(static_cast<int>(parsed)) == cleanRoot && // No leading zeros
                    parsed >= 0) {
                    isValidArrayIndex = true;
                    index = static_cast<int>(parsed);
                }
            }

            if (isValidArrayIndex && index <= opts.arrayLimit) {
                // Create sparse array
                if (opts.throwOnLimitExceeded && index >= opts.arrayLimit) {
                    throw RangeError("Array limit exceeded. Only " +
                        std::to_string(opts.arrayLimit) + " element" +
                        (opts.arrayLimit == 1 ? "" : "s") +
                        " allowed in an array.");
                }
                JsonArray arr;
                arr.resize(index + 1);
                arr[index] = std::move(leaf);
                leaf = JsonValue(std::move(arr));
            } else if (isValidArrayIndex && index > opts.arrayLimit) {
                // Overflow: create object with numeric string key
                if (opts.throwOnLimitExceeded) {
                    throw RangeError("Array limit exceeded. Only " +
                        std::to_string(opts.arrayLimit) + " element" +
                        (opts.arrayLimit == 1 ? "" : "s") +
                        " allowed in an array.");
                }
                obj[cleanRoot] = std::move(leaf);
                leaf = JsonValue(std::move(obj));
            } else if (cleanRoot != "__proto__") {
                // Regular object key (silently drop __proto__)
                obj[cleanRoot] = std::move(leaf);
                leaf = JsonValue(std::move(obj));
            }
        }
    }

    return leaf;
}

/**
 * @brief Phase 1: Split query string into flat key-value pairs.
 *
 * Splits on the configured delimiter, decodes percent-encoded characters,
 * handles duplicate keys, and optionally splits comma-separated values.
 *
 * @param str The query string.
 * @param opts Parse options.
 * @return A JsonObject with flat (un-nested) key-value pairs.
 * @throws polycpp::RangeError If throwOnLimitExceeded and limits exceeded.
 * @since 0.1.0
 */
inline JsonObject parseValues(const std::string& str, const ParseOptions& opts) {
    JsonObject result;

    std::string cleanStr = str;

    // Strip leading ? if configured
    if (opts.ignoreQueryPrefix && !cleanStr.empty() && cleanStr[0] == '?') {
        cleanStr = cleanStr.substr(1);
    }

    if (cleanStr.empty()) return result;

    // Pre-decode brackets for key parsing: %5B -> [, %5D -> ]
    {
        std::string decoded;
        decoded.reserve(cleanStr.size());
        for (size_t i = 0; i < cleanStr.size(); ++i) {
            if (i + 2 < cleanStr.size() && cleanStr[i] == '%') {
                if ((cleanStr[i+1] == '5' || cleanStr[i+1] == '5') &&
                    (cleanStr[i+2] == 'B' || cleanStr[i+2] == 'b')) {
                    decoded += '[';
                    i += 2;
                    continue;
                }
                if ((cleanStr[i+1] == '5' || cleanStr[i+1] == '5') &&
                    (cleanStr[i+2] == 'D' || cleanStr[i+2] == 'd')) {
                    decoded += ']';
                    i += 2;
                    continue;
                }
            }
            decoded += cleanStr[i];
        }
        cleanStr = decoded;
    }

    // Split on delimiter
    int limit = opts.throwOnLimitExceeded ? opts.parameterLimit + 1 : opts.parameterLimit;
    auto parts = detail::split(cleanStr, opts.delimiter, limit);

    if (opts.throwOnLimitExceeded && static_cast<int>(parts.size()) > opts.parameterLimit) {
        throw RangeError("Parameter limit exceeded. Only " +
            std::to_string(opts.parameterLimit) + " parameter" +
            (opts.parameterLimit == 1 ? "" : "s") + " allowed.");
    }

    for (size_t i = 0; i < parts.size(); ++i) {
        const auto& part = parts[i];
        if (part.empty()) continue;

        // Find the = separator
        // Handle bracket-equals: a[>=]=23 should split as key="a[>=]", val="23"
        size_t bracketEqualsPos = part.find("]=");
        size_t eqPos;
        if (bracketEqualsPos != std::string::npos) {
            eqPos = bracketEqualsPos + 1;
        } else {
            eqPos = part.find('=');
        }

        std::string key;
        std::string val;
        if (eqPos == std::string::npos) {
            // No = sign
            key = detail::decode(part);
            if (opts.strictNullHandling) {
                val = ""; // Will be replaced with null below
            } else {
                val = "";
            }
        } else {
            key = detail::decode(part.substr(0, eqPos));
            val = detail::decode(part.substr(eqPos + 1));
        }

        // Handle comma splitting
        if (opts.comma && val.find(',') != std::string::npos) {
            auto commaParts = detail::split(val, ",", 0);
            JsonArray arr;
            for (auto& cp : commaParts) {
                arr.push_back(JsonValue(std::move(cp)));
            }
            // Store as array value
            auto it = result.find(key);
            if (it != result.end()) {
                // Duplicate key with comma: combine
                if (opts.duplicates == Duplicates::combine) {
                    it->second = detail::combine(it->second, JsonValue(std::move(arr)));
                } else if (opts.duplicates == Duplicates::last) {
                    it->second = JsonValue(std::move(arr));
                }
                // first: keep existing
            } else {
                result[key] = JsonValue(std::move(arr));
            }
            continue;
        }

        // Create the value
        JsonValue value;
        if (eqPos == std::string::npos && opts.strictNullHandling) {
            value = JsonValue(nullptr);
        } else {
            value = JsonValue(val);
        }

        // Handle duplicates
        auto it = result.find(key);
        if (it != result.end()) {
            if (opts.duplicates == Duplicates::combine) {
                // Check if key ends with [] - always combine
                bool hasBrackets = key.size() >= 2 &&
                    key.substr(key.size() - 2) == "[]";
                it->second = detail::combine(it->second, value);
            } else if (opts.duplicates == Duplicates::last) {
                it->second = value;
            }
            // first: keep existing
        } else {
            result[key] = value;
        }
    }

    return result;
}

/**
 * @brief Full parse implementation: parseValues -> parseKeys -> merge -> compact.
 * @since 0.1.0
 */
inline JsonValue parseImpl(const std::string& str, const ParseOptions& opts) {
    if (str.empty()) {
        return JsonValue(JsonObject{});
    }

    // Check for leading ? even if ignoreQueryPrefix is false
    std::string input = str;
    // parseValues handles ignoreQueryPrefix internally

    // Phase 1: flat key-value extraction
    JsonObject flatPairs = parseValues(input, opts);

    // Phase 2+3: for each flat key, parse into nested structure and merge
    JsonValue result(JsonObject{});

    for (const auto& [key, val] : flatPairs) {
        // Split key into segments and build nested structure
        auto segments = splitKeyIntoSegments(key, opts);
        if (segments.empty()) continue;

        JsonValue nested = parseObject(segments, val, opts);
        result = detail::merge(result, nested);
    }

    // Phase 4: compact sparse arrays (unless allowSparse)
    if (!opts.allowSparse) {
        result = detail::compact(result);
    }

    return result;
}

} // namespace detail
} // namespace qs
} // namespace polycpp
