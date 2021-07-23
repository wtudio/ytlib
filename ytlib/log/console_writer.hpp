#pragma once

#include <iostream>

#include "color_console.hpp"
#include "log.hpp"

namespace ytlib {

class ConsoleWriter {
 public:
  void Write(const LogData& data) {
    if (print_color_) {
#if defined(_WIN32)
      static const std::map<LOG_LEVEL, WORD> color_map = {
          {LOG_LEVEL::L_DEBUG, CC_DBG},
          {LOG_LEVEL::L_INFO, CC_INF},
          {LOG_LEVEL::L_WARN, CC_WRN},
          {LOG_LEVEL::L_ERROR, CC_ERR},
          {LOG_LEVEL::L_FATAL, CC_FATAL},
      };

      auto finditr = color_map.find(data.lvl);
      if (finditr != color_map.end()) {
        SetConsoleTextAttribute(console_handle_, finditr->second);
        WriteDirectly(data);
        SetConsoleTextAttribute(console_handle_, CC_DEFAULT);
      } else {
        WriteDirectly(data);
      }

#else
      static const std::map<LOG_LEVEL, std::string> color_map = {
          {LOG_LEVEL::L_DEBUG, CC_DBG},
          {LOG_LEVEL::L_INFO, CC_INF},
          {LOG_LEVEL::L_WARN, CC_WRN},
          {LOG_LEVEL::L_ERROR, CC_ERR},
          {LOG_LEVEL::L_FATAL, CC_FATAL},
      };

      auto finditr = color_map.find(data.lvl);
      if (finditr != color_map.end()) {
        std::cout << finditr->second;
        WriteDirectly(data);
        std::cout << CC_NONE;
      } else {
        WriteDirectly(data);
      }

#endif
    } else {
      WriteDirectly(data);
    }
    std::cout << std::endl;
  }
  void WriteDirectly(const LogData& data) {
    std::cout << "[" << data.time << "]["
              << static_cast<uint8_t>(data.lvl) << "]["
              << data.thread_id << "]";

    if (data.ctx) {
      for (auto& itr : *(data.ctx))
        std::cout << "[" << itr.first << "=" << itr.second << "]";
    }

    std::cout << data.msg;
  }

  bool print_color_ = true;

#if defined(_WIN32)
  bool SetConsole() {
    console_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle_ == INVALID_HANDLE_VALUE) return false;

    CONSOLE_FONT_INFOEX FontInfo;
    COORD dwSize;
    SetConsoleOutputCP(CP_UTF8);
    FontInfo.cbSize = sizeof(FontInfo);
    FontInfo.dwFontSize.X = 8;
    FontInfo.dwFontSize.Y = 16;
    wcsncpy(FontInfo.FaceName, L"Consolas", LF_FACESIZE);
    FontInfo.FontFamily = FF_DONTCARE;
    FontInfo.FontWeight = FW_BOLD;
    FontInfo.nFont = 1;
    SetCurrentConsoleFontEx(console_handle_, FALSE, &FontInfo);
    dwSize.X = 80;
    dwSize.Y = 1024;
    SetConsoleScreenBufferSize(console_handle_, dwSize);
    SetConsoleMode(console_handle_, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
    //HMENU hMenu = GetSystemMenu(GetConsoleWindow(), FALSE);
    //EnableMenuItem(hMenu, SC_CLOSE, MF_GRAYED);

    return true;
  }

  HANDLE console_handle_ = INVALID_HANDLE_VALUE;
#endif
};

inline LogWriter GetConsoleWriter(const std::map<std::string, std::string>& cfg) {
  auto pw = std::make_shared<ConsoleWriter>();

  auto finditr = cfg.find("color");
  if (finditr != cfg.end())
    pw->print_color_ = (finditr->second == "true");

#if defined(_WIN32)
  if (pw->print_color_)
    pw->SetConsole();

#endif

  fprintf(stderr, "init ConsoleWriter, color:%s\n", (pw->print_color_) ? "true" : "false");

  return [pw](const LogData& data) {
    pw->Write(data);
  };
}

}  // namespace ytlib
