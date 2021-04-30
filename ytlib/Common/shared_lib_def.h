/**
 * @file shared_lib_def.h
 * @brief 动态库头文件
 * @details 定义YT_DECLSPEC_IMPORT、YT_DECLSPEC_IMPORT宏
 * @author WT
 * @email 905976782@qq.com
 * @date 2021-04-30
 */
#pragma once

// 导出定义
#if defined(BUILD_SHARED_LIBS)
  #if defined(_WIN32)
    #define YT_DECLSPEC_EXPORT __declspec(dllexport)
    #define YT_DECLSPEC_IMPORT __declspec(dllimport)
  #else
    #define YT_DECLSPEC_EXPORT __attribute__((visibility("default")))
    #define YT_DECLSPEC_IMPORT
  #endif
#else
  #define YT_DECLSPEC_EXPORT
  #define YT_DECLSPEC_IMPORT
#endif