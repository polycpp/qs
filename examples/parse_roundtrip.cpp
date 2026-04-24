// parse_roundtrip — parse a query string, print the JsonValue tree,
// then stringify it back and confirm the result parses to the same
// shape.  Useful for eyeballing how qs maps bracket / dot notation
// onto the polycpp JSON value model.
//
// Usage:  parse_roundtrip 'a[b]=1&a[c]=2&tags[]=c&tags[]=cpp'
#include <iostream>
#include <string>

#include <polycpp/core/json.hpp>
#include <polycpp/qs/qs.hpp>

namespace qs = polycpp::qs;
using polycpp::JsonValue;

namespace {

void print(const JsonValue& v, int indent = 0) {
    std::string pad(static_cast<size_t>(indent), ' ');
    if (v.isObject()) {
        std::cout << "{\n";
        for (const auto& [k, child] : v.asObject()) {
            std::cout << pad << "  " << k << ": ";
            print(child, indent + 2);
        }
        std::cout << pad << "}\n";
    } else if (v.isArray()) {
        std::cout << "[\n";
        for (const auto& child : v.asArray()) {
            std::cout << pad << "  ";
            print(child, indent + 2);
        }
        std::cout << pad << "]\n";
    } else if (v.isString()) {
        std::cout << '"' << v.asString() << "\"\n";
    } else {
        std::cout << "(other)\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <query-string>\n";
        return 64;
    }
    const std::string raw = argv[1];

    qs::ParseOptions popts;
    popts.ignoreQueryPrefix = true;
    auto parsed = qs::parse(raw, popts);

    std::cout << "-- parsed --\n";
    print(parsed);

    qs::StringifyOptions sopts;
    sopts.encode = false;
    std::cout << "-- stringified --\n" << qs::stringify(parsed, sopts) << '\n';

    auto reparsed = qs::parse(qs::stringify(parsed, sopts), popts);
    std::cout << "round-trip matches: "
              << (reparsed == parsed ? "yes" : "no") << '\n';
    return 0;
}
