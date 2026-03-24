#ifndef IO_H
#define IO_H

// io.hpp
#pragma once
#include <string>
#include <optional>
#include <tuple>

struct Range { long long start; long long end; }; // inclusive
std::optional<Range> parse_range(const std::string& header, long long file_size);

#endif // IO_H
