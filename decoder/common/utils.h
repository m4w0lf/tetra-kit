#ifndef UTILS_H
#define UTILS_H
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <vector>
#include <string>

/**
 * @defgroup tetra_utils Miscellaneous functions
 *
 * @{
 *
 */

namespace Tetra {

    std::vector<uint8_t> vectorExtract(std::vector<uint8_t> source, const uint32_t pos, const int32_t length); // extract sub-vector
    std::vector<uint8_t> vectorAppend(std::vector<uint8_t> vec1, std::vector<uint8_t> vec2);                   // concatenate vectors
    std::string vectorToString(const std::vector<uint8_t> data, const int len);
    std::string formatStr(const char * fmt, ...);

};

/** @} */

#endif /* UTILS_H */
