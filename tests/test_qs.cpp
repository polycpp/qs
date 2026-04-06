/**
 * @file test_qs.cpp
 * @brief Tests for polycpp::qs — parse and stringify.
 *
 * Test cases ported from npm qs test suite.
 */

#include <gtest/gtest.h>
#include <polycpp/qs/qs.hpp>

using namespace polycpp;
using namespace polycpp::qs;

// ============================================================================
// Parse Tests — Basic
// ============================================================================

TEST(QsParseTest, SimpleKeyValue) {
    auto result = parse("a=b");
    EXPECT_TRUE(result.isObject());
    EXPECT_EQ(result["a"].asString(), "b");
}

TEST(QsParseTest, MultipleKeyValues) {
    auto result = parse("a=b&c=d");
    EXPECT_EQ(result["a"].asString(), "b");
    EXPECT_EQ(result["c"].asString(), "d");
}

TEST(QsParseTest, EmptyString) {
    auto result = parse("");
    EXPECT_TRUE(result.isObject());
    EXPECT_TRUE(result.asObject().empty());
}

TEST(QsParseTest, NoValueDefaultsToEmptyString) {
    auto result = parse("a");
    EXPECT_EQ(result["a"].asString(), "");
}

TEST(QsParseTest, StrictNullHandlingNoValueIsNull) {
    ParseOptions opts;
    opts.strictNullHandling = true;
    auto result = parse("a", opts);
    EXPECT_TRUE(result["a"].isNull());
}

TEST(QsParseTest, IgnoreQueryPrefix) {
    ParseOptions opts;
    opts.ignoreQueryPrefix = true;
    auto result = parse("?a=b&c=d", opts);
    EXPECT_EQ(result["a"].asString(), "b");
    EXPECT_EQ(result["c"].asString(), "d");
}

TEST(QsParseTest, PercentDecodedValues) {
    auto result = parse("a=hello%20world");
    EXPECT_EQ(result["a"].asString(), "hello world");
}

TEST(QsParseTest, PlusDecodedAsSpace) {
    auto result = parse("a=hello+world");
    EXPECT_EQ(result["a"].asString(), "hello world");
}

TEST(QsParseTest, PercentEncodedSpecialChars) {
    auto result = parse("a=%26%3D%23");
    EXPECT_EQ(result["a"].asString(), "&=#");
}

// ============================================================================
// Parse Tests — Nested Objects
// ============================================================================

TEST(QsParseTest, BracketNotation) {
    auto result = parse("a[b]=c");
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_EQ(result["a"]["b"].asString(), "c");
}

TEST(QsParseTest, DeepBracketNotation) {
    auto result = parse("a[b][c]=d");
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_TRUE(result["a"]["b"].isObject());
    EXPECT_EQ(result["a"]["b"]["c"].asString(), "d");
}

TEST(QsParseTest, MultipleNestedKeys) {
    auto result = parse("a[b]=1&a[c]=2");
    EXPECT_EQ(result["a"]["b"].asString(), "1");
    EXPECT_EQ(result["a"]["c"].asString(), "2");
}

TEST(QsParseTest, DotNotation) {
    ParseOptions opts;
    opts.allowDots = true;
    auto result = parse("a.b.c=d", opts);
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_TRUE(result["a"]["b"].isObject());
    EXPECT_EQ(result["a"]["b"]["c"].asString(), "d");
}

TEST(QsParseTest, MixedDotAndBracketNotation) {
    ParseOptions opts;
    opts.allowDots = true;
    auto result = parse("a.b[c]=d", opts);
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_TRUE(result["a"]["b"].isObject());
    EXPECT_EQ(result["a"]["b"]["c"].asString(), "d");
}

// ============================================================================
// Parse Tests — Arrays
// ============================================================================

TEST(QsParseTest, ArrayIndexNotation) {
    auto result = parse("a[0]=b&a[1]=c");
    EXPECT_TRUE(result["a"].isArray());
    const auto& arr = result["a"].asArray();
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[0].asString(), "b");
    EXPECT_EQ(arr[1].asString(), "c");
}

TEST(QsParseTest, EmptyBracketsCreateArray) {
    auto result = parse("a[]=b&a[]=c");
    EXPECT_TRUE(result["a"].isArray());
    const auto& arr = result["a"].asArray();
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[0].asString(), "b");
    EXPECT_EQ(arr[1].asString(), "c");
}

TEST(QsParseTest, SparseArrayCompaction) {
    auto result = parse("a[1]=b&a[5]=c");
    EXPECT_TRUE(result["a"].isArray());
    const auto& arr = result["a"].asArray();
    // After compaction, should remove null holes
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[0].asString(), "b");
    EXPECT_EQ(arr[1].asString(), "c");
}

TEST(QsParseTest, SparseArrayPreservedWithAllowSparse) {
    ParseOptions opts;
    opts.allowSparse = true;
    auto result = parse("a[1]=b&a[5]=c", opts);
    EXPECT_TRUE(result["a"].isArray());
    const auto& arr = result["a"].asArray();
    EXPECT_EQ(arr.size(), 6u);
    EXPECT_TRUE(arr[0].isNull());
    EXPECT_EQ(arr[1].asString(), "b");
    EXPECT_TRUE(arr[2].isNull());
    EXPECT_EQ(arr[5].asString(), "c");
}

TEST(QsParseTest, ArrayLimitExceededBecomesObject) {
    ParseOptions opts;
    opts.arrayLimit = 2;
    auto result = parse("a[5]=b", opts);
    // Index 5 > arrayLimit 2, so becomes object
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_EQ(result["a"]["5"].asString(), "b");
}

TEST(QsParseTest, ParseArraysFalse) {
    ParseOptions opts;
    opts.parseArrays = false;
    auto result = parse("a[0]=b&a[1]=c", opts);
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_EQ(result["a"]["0"].asString(), "b");
    EXPECT_EQ(result["a"]["1"].asString(), "c");
}

TEST(QsParseTest, AllowEmptyArrays) {
    ParseOptions opts;
    opts.allowEmptyArrays = true;
    auto result = parse("foo[]", opts);
    EXPECT_TRUE(result["foo"].isArray());
    EXPECT_TRUE(result["foo"].asArray().empty());
}

// ============================================================================
// Parse Tests — Comma Splitting
// ============================================================================

TEST(QsParseTest, CommaSplitting) {
    ParseOptions opts;
    opts.comma = true;
    auto result = parse("a=1,2,3", opts);
    EXPECT_TRUE(result["a"].isArray());
    const auto& arr = result["a"].asArray();
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0].asString(), "1");
    EXPECT_EQ(arr[1].asString(), "2");
    EXPECT_EQ(arr[2].asString(), "3");
}

TEST(QsParseTest, CommaSplittingDisabledByDefault) {
    auto result = parse("a=1,2,3");
    EXPECT_TRUE(result["a"].isString());
    EXPECT_EQ(result["a"].asString(), "1,2,3");
}

// ============================================================================
// Parse Tests — Depth Limit
// ============================================================================

TEST(QsParseTest, DepthLimit) {
    ParseOptions opts;
    opts.depth = 1;
    auto result = parse("a[b][c]=d", opts);
    EXPECT_TRUE(result["a"].isObject());
    // With depth=1: parent "a" + 1 bracket "[b]", "[c]" becomes literal key
    EXPECT_TRUE(result["a"]["b"].isObject());
    EXPECT_EQ(result["a"]["b"]["[c]"].asString(), "d");
}

TEST(QsParseTest, StrictDepthThrows) {
    ParseOptions opts;
    opts.depth = 1;
    opts.strictDepth = true;
    EXPECT_THROW(parse("a[b][c][d]=e", opts), RangeError);
}

TEST(QsParseTest, DepthZeroFlatKeys) {
    ParseOptions opts;
    opts.depth = 0;
    auto result = parse("a[b]=c", opts);
    // With depth 0, the entire key "a[b]" is treated as a flat key
    EXPECT_EQ(result["a[b]"].asString(), "c");
}

// ============================================================================
// Parse Tests — Duplicate Keys
// ============================================================================

TEST(QsParseTest, DuplicateKeysCombine) {
    auto result = parse("a=1&a=2");
    EXPECT_TRUE(result["a"].isArray());
    const auto& arr = result["a"].asArray();
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[0].asString(), "1");
    EXPECT_EQ(arr[1].asString(), "2");
}

TEST(QsParseTest, DuplicateKeysFirst) {
    ParseOptions opts;
    opts.duplicates = Duplicates::first;
    auto result = parse("a=1&a=2", opts);
    EXPECT_EQ(result["a"].asString(), "1");
}

TEST(QsParseTest, DuplicateKeysLast) {
    ParseOptions opts;
    opts.duplicates = Duplicates::last;
    auto result = parse("a=1&a=2", opts);
    EXPECT_EQ(result["a"].asString(), "2");
}

// ============================================================================
// Parse Tests — Parameter Limit
// ============================================================================

TEST(QsParseTest, ParameterLimit) {
    ParseOptions opts;
    opts.parameterLimit = 2;
    auto result = parse("a=1&b=2&c=3", opts);
    EXPECT_EQ(result["a"].asString(), "1");
    EXPECT_EQ(result["b"].asString(), "2");
    // c should be absent (limited to 2 params)
    EXPECT_TRUE(result.asObject().find("c") == result.asObject().end());
}

TEST(QsParseTest, ThrowOnLimitExceeded) {
    ParseOptions opts;
    opts.parameterLimit = 2;
    opts.throwOnLimitExceeded = true;
    EXPECT_THROW(parse("a=1&b=2&c=3", opts), RangeError);
}

// ============================================================================
// Parse Tests — Custom Delimiter
// ============================================================================

TEST(QsParseTest, CustomDelimiter) {
    ParseOptions opts;
    opts.delimiter = ";";
    auto result = parse("a=1;b=2;c=3", opts);
    EXPECT_EQ(result["a"].asString(), "1");
    EXPECT_EQ(result["b"].asString(), "2");
    EXPECT_EQ(result["c"].asString(), "3");
}

// ============================================================================
// Parse Tests — Proto Pollution Prevention
// ============================================================================

TEST(QsParseTest, ProtoKeyDropped) {
    auto result = parse("__proto__[x]=bad");
    // __proto__ keys should be silently dropped
    EXPECT_TRUE(result.asObject().find("__proto__") == result.asObject().end());
}

// ============================================================================
// Stringify Tests — Basic
// ============================================================================

TEST(QsStringifyTest, SimpleKeyValue) {
    JsonValue obj = JsonObject{{"a", "b"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=b");
}

TEST(QsStringifyTest, MultipleKeyValues) {
    JsonValue obj = JsonObject{{"a", "b"}, {"c", "d"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=b&c=d");
}

TEST(QsStringifyTest, EmptyObject) {
    JsonValue obj = JsonObject{};
    auto result = stringify(obj);
    EXPECT_EQ(result, "");
}

TEST(QsStringifyTest, NonObjectReturnsEmpty) {
    auto result = stringify(JsonValue("hello"));
    EXPECT_EQ(result, "");
}

TEST(QsStringifyTest, AddQueryPrefix) {
    JsonValue obj = JsonObject{{"a", "b"}};
    StringifyOptions opts;
    opts.addQueryPrefix = true;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "?a=b");
}

TEST(QsStringifyTest, PercentEncodedValues) {
    JsonValue obj = JsonObject{{"a", "hello world"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=hello%20world");
}

TEST(QsStringifyTest, RFC1738Format) {
    JsonValue obj = JsonObject{{"a", "hello world"}};
    StringifyOptions opts;
    opts.format = Format::RFC1738;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a=hello+world");
}

// ============================================================================
// Stringify Tests — Nested Objects
// ============================================================================

TEST(QsStringifyTest, NestedObject) {
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b", "c"}}}
    };
    auto result = stringify(obj);
    EXPECT_EQ(result, "a%5Bb%5D=c");
}

TEST(QsStringifyTest, DeepNestedObject) {
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b", JsonObject{{"c", "d"}}}}}
    };
    auto result = stringify(obj);
    EXPECT_EQ(result, "a%5Bb%5D%5Bc%5D=d");
}

TEST(QsStringifyTest, DotNotation) {
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b", "c"}}}
    };
    StringifyOptions opts;
    opts.allowDots = true;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a.b=c");
}

// ============================================================================
// Stringify Tests — Arrays
// ============================================================================

TEST(QsStringifyTest, ArrayIndicesFormat) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{"b", "c"}}
    };
    auto result = stringify(obj);
    EXPECT_EQ(result, "a%5B0%5D=b&a%5B1%5D=c");
}

TEST(QsStringifyTest, ArrayBracketsFormat) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{"b", "c"}}
    };
    StringifyOptions opts;
    opts.arrayFormat = ArrayFormat::brackets;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a%5B%5D=b&a%5B%5D=c");
}

TEST(QsStringifyTest, ArrayRepeatFormat) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{"b", "c"}}
    };
    StringifyOptions opts;
    opts.arrayFormat = ArrayFormat::repeat;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a=b&a=c");
}

TEST(QsStringifyTest, ArrayCommaFormat) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{"b", "c"}}
    };
    StringifyOptions opts;
    opts.arrayFormat = ArrayFormat::comma;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a=b%2Cc");
}

TEST(QsStringifyTest, EmptyArrayOmitted) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{}}
    };
    auto result = stringify(obj);
    EXPECT_EQ(result, "");
}

TEST(QsStringifyTest, EmptyArrayWithAllowEmptyArrays) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{}}
    };
    StringifyOptions opts;
    opts.allowEmptyArrays = true;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a[]");
}

// ============================================================================
// Stringify Tests — Null Handling
// ============================================================================

TEST(QsStringifyTest, NullValueEmptyString) {
    JsonValue obj = JsonObject{{"a", nullptr}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=");
}

TEST(QsStringifyTest, StrictNullHandling) {
    JsonValue obj = JsonObject{{"a", nullptr}};
    StringifyOptions opts;
    opts.strictNullHandling = true;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a");
}

TEST(QsStringifyTest, SkipNulls) {
    JsonValue obj = JsonObject{{"a", "b"}, {"c", nullptr}};
    StringifyOptions opts;
    opts.skipNulls = true;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a=b");
}

// ============================================================================
// Stringify Tests — Numbers and Booleans
// ============================================================================

TEST(QsStringifyTest, NumberValue) {
    JsonValue obj = JsonObject{{"a", 42}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=42");
}

TEST(QsStringifyTest, FloatValue) {
    JsonValue obj = JsonObject{{"a", 3.14}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=3.14");
}

TEST(QsStringifyTest, BooleanValue) {
    JsonValue obj = JsonObject{{"a", true}, {"b", false}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=true&b=false");
}

// ============================================================================
// Stringify Tests — Encoding
// ============================================================================

TEST(QsStringifyTest, EncodeDisabled) {
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b", "c d"}}}
    };
    StringifyOptions opts;
    opts.encode = false;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a[b]=c d");
}

TEST(QsStringifyTest, EncodeValuesOnly) {
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b", "c d"}}}
    };
    StringifyOptions opts;
    opts.encodeValuesOnly = true;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a[b]=c%20d");
}

// ============================================================================
// Stringify Tests — Custom Delimiter
// ============================================================================

TEST(QsStringifyTest, CustomDelimiter) {
    JsonValue obj = JsonObject{{"a", "b"}, {"c", "d"}};
    StringifyOptions opts;
    opts.delimiter = ";";
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a=b;c=d");
}

// ============================================================================
// Round-trip Tests
// ============================================================================

TEST(QsRoundTripTest, SimpleValues) {
    auto parsed = parse("a=b&c=d");
    auto str = stringify(parsed);
    auto reparsed = parse(str);
    EXPECT_EQ(reparsed["a"].asString(), "b");
    EXPECT_EQ(reparsed["c"].asString(), "d");
}

TEST(QsRoundTripTest, NestedObjects) {
    StringifyOptions sopts;
    sopts.encode = false;
    auto obj = parse("a[b][c]=d");
    auto str = stringify(obj, sopts);
    auto reparsed = parse(str);
    EXPECT_EQ(reparsed["a"]["b"]["c"].asString(), "d");
}

TEST(QsRoundTripTest, ArraysWithIndices) {
    auto parsed = parse("a[0]=b&a[1]=c");
    StringifyOptions sopts;
    sopts.encode = false;
    auto str = stringify(parsed, sopts);
    auto reparsed = parse(str);
    EXPECT_TRUE(reparsed["a"].isArray());
    EXPECT_EQ(reparsed["a"].asArray()[0].asString(), "b");
    EXPECT_EQ(reparsed["a"].asArray()[1].asString(), "c");
}

TEST(QsRoundTripTest, DotNotation) {
    ParseOptions popts;
    popts.allowDots = true;
    StringifyOptions sopts;
    sopts.allowDots = true;
    sopts.encode = false;

    auto parsed = parse("a.b.c=d", popts);
    auto str = stringify(parsed, sopts);
    auto reparsed = parse(str, popts);
    EXPECT_EQ(reparsed["a"]["b"]["c"].asString(), "d");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(QsEdgeCaseTest, EncodedBracketsInInput) {
    // %5B and %5D should be treated as [ and ]
    auto result = parse("a%5Bb%5D=c");
    EXPECT_TRUE(result["a"].isObject());
    EXPECT_EQ(result["a"]["b"].asString(), "c");
}

TEST(QsEdgeCaseTest, EmptyValue) {
    auto result = parse("a=");
    EXPECT_EQ(result["a"].asString(), "");
}

TEST(QsEdgeCaseTest, EqualsInValue) {
    auto result = parse("a=b=c");
    EXPECT_EQ(result["a"].asString(), "b=c");
}

TEST(QsEdgeCaseTest, SpecialCharsInValue) {
    auto result = parse("a=%26%3D%23%2F");
    EXPECT_EQ(result["a"].asString(), "&=#/");
}

TEST(QsEdgeCaseTest, UnicodeValues) {
    // UTF-8 encoded unicode
    auto result = parse("a=%E4%B8%AD%E6%96%87");
    // Should decode to Chinese characters
    EXPECT_FALSE(result["a"].asString().empty());
}

TEST(QsEdgeCaseTest, LeadingAmpersand) {
    auto result = parse("&a=b");
    EXPECT_EQ(result["a"].asString(), "b");
}

TEST(QsEdgeCaseTest, TrailingAmpersand) {
    auto result = parse("a=b&");
    EXPECT_EQ(result["a"].asString(), "b");
}

TEST(QsEdgeCaseTest, MultipleAmpersands) {
    auto result = parse("a=b&&c=d");
    EXPECT_EQ(result["a"].asString(), "b");
    EXPECT_EQ(result["c"].asString(), "d");
}

TEST(QsEdgeCaseTest, StringifyThenParsePreservesTypes) {
    JsonValue obj = JsonObject{
        {"str", "hello"},
        {"nested", JsonObject{{"key", "val"}}},
    };
    auto str = stringify(obj);
    auto parsed = parse(str);
    // Note: all values become strings after parse (no type inference)
    EXPECT_EQ(parsed["str"].asString(), "hello");
    EXPECT_TRUE(parsed["nested"].isObject());
    EXPECT_EQ(parsed["nested"]["key"].asString(), "val");
}

TEST(QsEdgeCaseTest, CommaRoundTrip) {
    JsonValue obj = JsonObject{
        {"a", JsonArray{"b"}}
    };
    StringifyOptions sopts;
    sopts.arrayFormat = ArrayFormat::comma;
    sopts.commaRoundTrip = true;
    sopts.encode = false;
    auto str = stringify(obj, sopts);
    // Single-element with commaRoundTrip should use bracket notation
    EXPECT_EQ(str, "a[]=b");
}

// ============================================================================
// encodeDotInKeys Tests
// ============================================================================

TEST(QsStringifyTest, EncodeDotInKeysBasic) {
    // Keys containing dots should have them encoded as %2E (not %252E)
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b.c", "d"}}}
    };
    StringifyOptions opts;
    opts.allowDots = true;
    opts.encodeDotInKeys = true;
    opts.encode = false;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a.b%2Ec=d");
}

TEST(QsStringifyTest, EncodeDotInKeysWithEncodeEnabled) {
    // Dots must become %2E, not double-encoded to %252E
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b.c", "d e"}}}
    };
    StringifyOptions opts;
    opts.allowDots = true;
    opts.encodeDotInKeys = true;
    // encode defaults to true
    auto result = stringify(obj, opts);
    // Key dot -> %2E, value space -> %20, bracket chars from key encoding
    EXPECT_NE(result.find("%2E"), std::string::npos) << "dot should be %2E";
    EXPECT_EQ(result.find("%252E"), std::string::npos) << "must not double-encode";
}

TEST(QsStringifyTest, EncodeDotInKeysRoundTrip) {
    // Stringify with encodeDotInKeys produces %2E for in-key dots
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b.c", "d"}}}
    };
    StringifyOptions sopts;
    sopts.allowDots = true;
    sopts.encodeDotInKeys = true;
    sopts.encode = false;
    auto str = stringify(obj, sopts);
    EXPECT_EQ(str, "a.b%2Ec=d") << "stringify should produce a.b%2Ec=d";

    // Parse with decodeDotInKeys: %2E is decoded to . BEFORE dot-splitting,
    // so a.b%2Ec becomes a.b.c (three nesting levels). This matches npm qs.
    ParseOptions popts;
    popts.allowDots = true;
    popts.decodeDotInKeys = true;
    auto parsed = parse(str, popts);
    EXPECT_TRUE(parsed.isObject());
    EXPECT_TRUE(parsed.hasKey("a")) << "should have top-level key 'a'";
    EXPECT_TRUE(parsed["a"].isObject()) << "a should be an object";
    EXPECT_TRUE(parsed["a"].hasKey("b")) << "a should have key 'b'";
    EXPECT_TRUE(parsed["a"]["b"].isObject()) << "a.b should be an object";
    EXPECT_EQ(parsed["a"]["b"]["c"].asString(), "d");
}

// ============================================================================
// Unreserved Character Encoding Tests
// ============================================================================

TEST(QsStringifyTest, ExclamationMarkEncoded) {
    // npm qs encodes ! ' * (they are not in the unreserved set)
    JsonValue obj = JsonObject{{"a", "hello!"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=hello%21");
}

TEST(QsStringifyTest, ApostropheEncoded) {
    JsonValue obj = JsonObject{{"a", "it's"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=it%27s");
}

TEST(QsStringifyTest, AsteriskEncoded) {
    JsonValue obj = JsonObject{{"a", "wild*card"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=wild%2Acard");
}

TEST(QsStringifyTest, RFC1738ParenthesesNotEncoded) {
    // RFC1738 format additionally leaves ( ) unencoded
    JsonValue obj = JsonObject{{"a", "f(x)"}};
    StringifyOptions opts;
    opts.format = Format::RFC1738;
    auto result = stringify(obj, opts);
    EXPECT_EQ(result, "a=f(x)");
}

TEST(QsStringifyTest, RFC3986ParenthesesEncoded) {
    // RFC3986 format DOES encode ( )
    JsonValue obj = JsonObject{{"a", "f(x)"}};
    auto result = stringify(obj);
    EXPECT_EQ(result, "a=f%28x%29");
}

TEST(QsStringifyTest, UnreservedCharsRoundTrip) {
    // Values with ! ' * should survive stringify -> parse round-trip
    auto str = stringify(JsonValue(JsonObject{{"a", "a!b'c*d"}}));
    auto parsed = parse(str);
    EXPECT_EQ(parsed["a"].asString(), "a!b'c*d");
}

TEST(QsStringifyTest, EncodeDotInKeysSentinelNotInValue) {
    // Byte \x01 in a value must not be corrupted by the encodeDotInKeys
    // sentinel post-processing — only key portions should be patched.
    std::string val(1, '\x01'); // single byte 0x01
    JsonValue obj = JsonObject{
        {"a", JsonObject{{"b.c", val}}}
    };
    StringifyOptions opts;
    opts.allowDots = true;
    opts.encodeDotInKeys = true;
    // encode=true: \x01 in the value becomes %01
    auto result = stringify(obj, opts);
    // Key dot -> %2E, value \x01 -> %01 (must NOT become %2E)
    EXPECT_NE(result.find("=%01"), std::string::npos)
        << "value's %01 must survive; got: " << result;
}
