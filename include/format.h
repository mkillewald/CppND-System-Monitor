#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
template <typename T>
std::string ZeroedString(T value);
std::string ElapsedTime(long times);
};  // namespace Format

#endif