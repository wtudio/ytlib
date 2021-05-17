/**
 * @file Util.h
 * @brief 基础头文件
 * @details ytlib基础头文件，提供一些基础工具与常用宏定义
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

// 默认windows下使用unicode，其他平台不使用。在这里添加要使用unicode的平台宏定义
#if defined(_WIN32)
  #ifndef UNICODE
    #define UNICODE
  #endif
#endif

// 包含常用头文件
#include <cassert>
#include <iostream>

// 包含特定头文件
#if defined(_WIN32)
  #define NOMINMAX
  #include <Shlwapi.h>
  #include <WinSock2.h>
  #include <Windows.h>
#else
  #include <arpa/inet.h>
  #include <dlfcn.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
#endif

// 定义DEBUG
#if defined(_DEBUG) || defined(DEBUG) || defined(__DEBUG__)
  #ifndef DEBUG
    #define DEBUG
  #endif
#endif

// 关闭警告
#if defined(_MSC_VER)
  #pragma warning(disable : 4068)
  #pragma warning(disable : 4251)
  #pragma warning(disable : 4275)
  #pragma warning(disable : 4290)
  #pragma warning(disable : 4819)
  #pragma warning(disable : 4996)

  #define _CRT_SECURE_NO_DEPRECATE

#endif

// 定义NULL
#if !defined(NULL)
  #if defined(__cplusplus)
    #define NULL 0
  #else
    #define NULL ((void *)0)
  #endif
#endif

#define YT_DEBUG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
