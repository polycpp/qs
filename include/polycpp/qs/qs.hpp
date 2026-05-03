#pragma once

/**
 * @file qs/qs.hpp
 * @brief C++ port of npm qs — query string parsing and stringifying.
 *
 * Provides `parse()` to convert URL query strings into nested `JsonValue`
 * objects, and `stringify()` to convert `JsonValue` objects back into query
 * strings. Supports nested object/array notation (bracket and dot syntax),
 * configurable depth limits, array handling modes, and percent-encoding.
 *
 * @par Example
 * @code{.cpp}
 *   #include <polycpp/qs/qs.hpp>
 *
 *   // Parse a query string
 *   auto result = polycpp::qs::parse("a[b]=1&a[c]=2");
 *   // result == { "a": { "b": "1", "c": "2" } }
 *
 *   // Stringify an object
 *   polycpp::JsonObject obj = {{"a", polycpp::JsonObject{{"b", "1"}, {"c", "2"}}}};
 *   auto str = polycpp::qs::stringify(polycpp::JsonValue(obj));
 *   // str == "a%5Bb%5D=1&a%5Bc%5D=2"
 * @endcode
 *
 * @see https://www.npmjs.com/package/qs
 * @since 1.0.0
 */

#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include <polycpp/core/json.hpp>
#include <polycpp/core/error.hpp>

namespace polycpp {
class Date;
namespace buffer {
class Buffer;
}

namespace qs {

/**
 * @brief Duplicate key handling strategy during parsing.
 * @since 1.0.0
 */
enum class Duplicates {
    combine, ///< Combine duplicate keys into an array (default).
    first,   ///< Keep only the first occurrence.
    last     ///< Keep only the last occurrence.
};

/**
 * @brief Array serialization format for stringify.
 * @since 1.0.0
 */
enum class ArrayFormat {
    indices,  ///< `a[0]=b&a[1]=c` (default).
    brackets, ///< `a[]=b&a[]=c`
    repeat,   ///< `a=b&a=c`
    comma     ///< `a=b,c`
};

/**
 * @brief URI encoding format.
 * @since 1.0.0
 */
enum class Format {
    RFC3986, ///< Spaces as `%20` (default).
    RFC1738  ///< Spaces as `+`.
};

/**
 * @brief Whether an encoder/decoder call is processing a key or value.
 * @since 1.0.0
 */
enum class ComponentKind {
    key,  ///< Query string key component.
    value ///< Query string value component.
};

/**
 * @brief Context passed to custom parse decoders.
 * @since 1.0.0
 */
struct DecodeContext {
    std::string charset = "utf-8";           ///< Effective charset for this parse.
    ComponentKind kind = ComponentKind::value; ///< Current component kind.
};

/**
 * @brief Context passed to custom stringify encoders.
 * @since 1.0.0
 */
struct EncodeContext {
    std::string charset = "utf-8";           ///< Charset requested by stringify.
    ComponentKind kind = ComponentKind::value; ///< Current component kind.
    Format format = Format::RFC3986;         ///< URI encoding format.
};

/// @brief Custom parse decoder callback.
using Decoder = std::function<JsonValue(const std::string&, const DecodeContext&)>;

/// @brief Custom stringify encoder callback.
using Encoder = std::function<std::string(const std::string&, const EncodeContext&)>;

/// @brief Optional post-encoder formatter callback.
using Formatter = std::function<std::string(const std::string&)>;

/// @brief Stringify filter/replacer callback; `std::nullopt` omits a value.
using Filter = std::function<std::optional<JsonValue>(const std::string&, const JsonValue&)>;

/// @brief Object-key comparator used by stringify.
using Sort = std::function<bool(const std::string&, const std::string&)>;

/// @brief Date serializer used by `toQsValue(Date, opts)`.
using DateSerializer = std::function<JsonValue(const Date&)>;

/**
 * @brief Options controlling query string parsing behavior.
 *
 * Mirrors the parse options from npm qs. JavaScript-specific object-shape
 * options (`allowPrototypes`, `plainObjects`) are omitted because they address
 * JavaScript prototype behavior that is not present in `JsonValue`.
 *
 * @see https://github.com/ljharb/qs#parsing
 * @since 1.0.0
 */
struct ParseOptions {
    bool allowDots = false;          ///< Enable dot notation: `a.b=c` -> `{a:{b:"c"}}`.
    bool allowEmptyArrays = false;   ///< `foo[]` with no value -> `{foo:[]}`.
    bool allowSparse = false;        ///< Preserve sparse arrays (skip compaction).
    int arrayLimit = 20;             ///< Max array length; index >= this becomes an object key.
    std::string charset = "utf-8";   ///< `"utf-8"` or `"iso-8859-1"`.
    bool charsetSentinel = false;    ///< Let an upstream `utf8=` sentinel select the effective charset.
    bool comma = false;              ///< Split values on commas: `a=1,2` -> `{a:["1","2"]}`.
    bool decodeDotInKeys = false;    ///< Preserve double-encoded in-key dots; implies allowDots.
    Decoder decoder = nullptr;       ///< Optional custom decoder for raw key/value components.
    std::string delimiter = "&";     ///< Key-value pair separator.
    int depth = 5;                   ///< Max nesting depth for bracket/dot parsing.
    Duplicates duplicates = Duplicates::combine; ///< Handling for duplicate keys.
    bool ignoreQueryPrefix = false;  ///< Strip leading `?`.
    bool interpretNumericEntities = false; ///< Decode `&#NNNN;` in ISO-8859-1 values.
    int parameterLimit = 1000;       ///< Max number of key-value pairs to parse.
    bool parseArrays = true;         ///< Enable array index parsing (`a[0]=b`).
    bool strictDepth = false;        ///< Throw on depth limit exceeded.
    bool strictNullHandling = false; ///< Keys without `=` produce null instead of `""`.
    bool throwOnLimitExceeded = false; ///< Throw on parameterLimit/arrayLimit exceeded.
};

/**
 * @brief Options controlling query string serialization behavior.
 *
 * Mirrors the stringify options from npm qs for values representable by
 * `JsonValue`. Date/Buffer-style objects can be adapted with `toQsValue()`.
 *
 * @see https://github.com/ljharb/qs#stringifying
 * @since 1.0.0
 */
struct StringifyOptions {
    bool addQueryPrefix = false;         ///< Prepend `?` to output.
    bool allowDots = false;              ///< Use dot notation: `{a:{b:"c"}}` -> `a.b=c`.
    bool allowEmptyArrays = false;       ///< Emit `key[]` for empty arrays.
    ArrayFormat arrayFormat = ArrayFormat::indices; ///< Array serialization format.
    std::string charset = "utf-8";       ///< `"utf-8"` or `"iso-8859-1"`.
    bool charsetSentinel = false;        ///< Prepend an upstream-compatible `utf8=` charset sentinel.
    bool commaRoundTrip = false;         ///< With comma format, emit `a[]=c` for single-element arrays.
    std::string delimiter = "&";         ///< Key-value pair separator.
    bool encode = true;                  ///< Enable percent-encoding.
    bool encodeDotInKeys = false;        ///< Encode nested in-key dots for decodeDotInKeys round-trips.
    Encoder encoder = nullptr;           ///< Optional custom encoder for key/value components.
    bool encodeValuesOnly = false;       ///< Only encode values, leave keys as-is.
    std::optional<bool> indices = std::nullopt; ///< Deprecated qs compatibility: false selects repeat arrays.
    Filter filter = nullptr;             ///< Optional filter/replacer; nullopt omits the value.
    std::optional<std::vector<std::string>> filterKeys = std::nullopt; ///< Optional key/index allow-list.
    Format format = Format::RFC3986;     ///< URI encoding format.
    Formatter formatter = nullptr;       ///< Optional post-encoder formatter.
    DateSerializer serializeDate = nullptr; ///< Optional serializer used by `toQsValue(Date, opts)`.
    bool skipNulls = false;              ///< Omit keys with null values.
    Sort sort = nullptr;                 ///< Optional object-key comparator.
    bool strictNullHandling = false;     ///< Emit `key` (no `=`) for null values.
};

/**
 * @brief Decode a raw query component using qs defaults.
 *
 * Custom decoders can call this helper to match the built-in percent decoding
 * and charset conversion before applying their own transformation.
 */
JsonValue defaultDecode(const std::string& str, const DecodeContext& ctx = {});

/**
 * @brief Encode a query component using qs defaults.
 *
 * Custom encoders can call this helper to reuse built-in percent encoding,
 * charset conversion, and RFC1738/RFC3986 spacing behavior.
 */
std::string defaultEncode(const std::string& str, const EncodeContext& ctx = {});

/**
 * @brief Adapt a polycpp Date for qs stringification.
 *
 * Uses `opts.serializeDate` when provided; otherwise uses Date's ISO string,
 * matching npm qs Date handling.
 */
JsonValue toQsValue(const Date& date, const StringifyOptions& opts = {});

/**
 * @brief Adapt a polycpp Buffer for qs stringification.
 *
 * Buffers stringify as their decoded string contents, matching npm qs Buffer
 * handling for Node buffers.
 */
JsonValue toQsValue(const buffer::Buffer& buffer, const std::string& encoding = "utf8");

// Generic toString adapter; documented in the advanced parity guide.
/// @cond
template <typename T>
requires (!std::same_as<std::decay_t<T>, JsonValue>)
    && requires(const T& t) { { t.toString() } -> std::convertible_to<std::string>; }
inline JsonValue toQsValue(const T& value) {
    return std::string(value.toString());
}
/// @endcond

/**
 * @brief Parse a URL query string into a nested JsonValue object.
 *
 * Splits the input on the configured delimiter, decodes percent-encoded
 * characters, and reconstructs nested objects/arrays from bracket or dot
 * notation in keys. The result is always a JsonObject at the top level.
 *
 * @param str The query string to parse (e.g., `"a=1&b[c]=2"`).
 * @param opts Parsing options.
 * @return A JsonValue (object) representing the parsed query parameters.
 * @throws polycpp::RangeError If `strictDepth` is true and depth is exceeded,
 *         or if `throwOnLimitExceeded` is true and limits are exceeded.
 *
 * @par Example
 * @code{.cpp}
 *   using namespace polycpp;
 *   auto result = qs::parse("foo=bar&baz[qux]=quux");
 *   // result["foo"].asString() == "bar"
 *   // result["baz"]["qux"].asString() == "quux"
 *
 *   auto nested = qs::parse("a[0]=b&a[1]=c");
 *   // nested["a"] is a JsonArray: ["b", "c"]
 *
 *   qs::ParseOptions opts;
 *   opts.allowDots = true;
 *   auto dotted = qs::parse("a.b.c=d", opts);
 *   // dotted["a"]["b"]["c"].asString() == "d"
 * @endcode
 *
 * @see https://github.com/ljharb/qs#parsing
 * @since 1.0.0
 */
JsonValue parse(const std::string& str, const ParseOptions& opts = {});

/**
 * @brief Serialize a JsonValue object into a URL query string.
 *
 * Recursively walks a top-level object tree and emits `key=value` pairs joined
 * by the configured delimiter. Nested objects are represented with bracket
 * or dot notation depending on options. Arrays are formatted according to
 * the `arrayFormat` setting. Non-object roots return an empty string.
 *
 * @param obj The root value to stringify. Object roots are traversed; non-object
 *        roots return an empty string.
 * @param opts Stringification options.
 * @return The query string (e.g., `"a=1&b%5Bc%5D=2"`).
 *
 * @par Example
 * @code{.cpp}
 *   using namespace polycpp;
 *   JsonValue obj = JsonObject{
 *       {"a", "b"},
 *       {"c", JsonObject{{"d", "e"}}}
 *   };
 *   auto str = qs::stringify(obj);
 *   // str == "a=b&c%5Bd%5D=e"
 *
 *   qs::StringifyOptions opts;
 *   opts.addQueryPrefix = true;
 *   opts.allowDots = true;
 *   auto dotted = qs::stringify(obj, opts);
 *   // dotted == "?a=b&c.d=e"
 * @endcode
 *
 * @see https://github.com/ljharb/qs#stringifying
 * @since 1.0.0
 */
std::string stringify(const JsonValue& obj, const StringifyOptions& opts = {});

} // namespace qs
} // namespace polycpp
