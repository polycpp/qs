/**
 * @file qs.cpp
 * @brief Compiled translation unit for polycpp::qs.
 * @since 1.0.0
 */

#include <polycpp/qs/detail/aggregator.hpp>
#include <polycpp/buffer.hpp>
#include <polycpp/core/date.hpp>

#include <utility>

polycpp::JsonValue polycpp::qs::parse(const std::string& str, const ParseOptions& opts) {
    return detail::parseImpl(str, opts);
}

std::string polycpp::qs::stringify(const JsonValue& obj, const StringifyOptions& opts) {
    return detail::stringifyImpl(obj, opts);
}

polycpp::JsonValue polycpp::qs::defaultDecode(
    const std::string& str,
    const DecodeContext& ctx)
{
    std::string decoded = detail::decode(str);
    if (ctx.charset == "iso-8859-1") {
        decoded = detail::latin1ToUtf8(decoded);
    }
    return JsonValue(std::move(decoded));
}

std::string polycpp::qs::defaultEncode(
    const std::string& str,
    const EncodeContext& ctx)
{
    if (ctx.charset == "iso-8859-1") {
        return detail::encode(detail::utf8ToLatin1(str), ctx.format);
    }
    return detail::encode(str, ctx.format);
}

polycpp::JsonValue polycpp::qs::toQsValue(
    const Date& date,
    const StringifyOptions& opts)
{
    if (opts.serializeDate) {
        return opts.serializeDate(date);
    }
    return date.toISOString();
}

polycpp::JsonValue polycpp::qs::toQsValue(
    const buffer::Buffer& buffer,
    const std::string& encoding)
{
    return buffer.toString(encoding);
}
