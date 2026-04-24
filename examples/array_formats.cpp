// array_formats — stringify the same array under every ArrayFormat.
//
// Prints the same ``JsonArray`` serialised four different ways so you
// can compare them side by side.  Mirrors the table in
// docs/sphinx/guides/pick-array-format.rst.
#include <iostream>

#include <polycpp/core/json.hpp>
#include <polycpp/qs/qs.hpp>

namespace qs = polycpp::qs;
using polycpp::JsonValue;
using polycpp::JsonObject;
using polycpp::JsonArray;

int main() {
    JsonValue doc = JsonObject{{"tag", JsonArray{"c", "cpp", "c#"}}};

    struct Case {
        const char*     name;
        qs::ArrayFormat fmt;
    };
    constexpr Case cases[] = {
        {"indices ",  qs::ArrayFormat::indices},
        {"brackets",  qs::ArrayFormat::brackets},
        {"repeat  ",  qs::ArrayFormat::repeat},
        {"comma   ",  qs::ArrayFormat::comma},
    };

    for (const auto& c : cases) {
        qs::StringifyOptions opts;
        opts.arrayFormat = c.fmt;
        opts.encode      = false;
        std::cout << c.name << "  " << qs::stringify(doc, opts) << '\n';
    }

    // The comma format loses the array shape for a single-element
    // array unless you set commaRoundTrip.
    JsonValue one = JsonObject{{"tag", JsonArray{"c"}}};
    qs::StringifyOptions tight{.arrayFormat = qs::ArrayFormat::comma,
                               .encode      = false};
    std::cout << "\ncomma 1elt      " << qs::stringify(one, tight) << '\n';

    tight.commaRoundTrip = true;
    std::cout << "comma 1elt rt   " << qs::stringify(one, tight) << '\n';
    return 0;
}
