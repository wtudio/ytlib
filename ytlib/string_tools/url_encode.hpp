/**
 * @file url_encode.hpp
 * @brief Url编解码
 * @details UrlEncode、UrlDecode
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <string>

#include "string_tools_exports.h"

namespace ytlib {

/**
 * @brief UrlEncode
 * @param str 待编码字符串
 * @param up 是否转码为大写字符
 * @return 转码后的结果字符串
 */
STRING_TOOLS_API std::string UrlEncode(const std::string& str, bool up = true);

/**
 * @brief UrlDecode
 * @param str 待解码字符串
 * @return 解码后的结果字符串
 */
STRING_TOOLS_API std::string UrlDecode(const std::string& str);

}  // namespace ytlib
