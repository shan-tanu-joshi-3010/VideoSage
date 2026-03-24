// io.cpp
#include "io.h"
#include <regex>
std::optional<Range> parse_range(const std::string& h, long long size) {
    // Example: "bytes=START-END"
    if (h.rfind("bytes=", 0) != 0) return std::nullopt;
    auto s = h.substr(6);
    auto dash = s.find('-');
    if (dash == std::string::npos) return std::nullopt;
    std::string a = s.substr(0, dash), b = s.substr(dash+1);
    long long start = a.empty() ? 0 : std::stoll(a);
    long long end   = b.empty() ? (size-1) : std::stoll(b);
    if (start < 0 || end < start || end >= size) return std::nullopt;
    return Range{start, end};
}
