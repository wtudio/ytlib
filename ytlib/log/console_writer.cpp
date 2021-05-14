#include "console_writer.hpp"
#include <iostream>

#if defined(_WIN32)
  #include <windows.h>

  #define CC_DEFAULT (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

  #define CC_DBG (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
  #define CC_INF (FOREGROUND_GREEN)
  #define CC_WRN (FOREGROUND_GREEN | FOREGROUND_RED)
  #define CC_ERR (FOREGROUND_BLUE | FOREGROUND_RED)
  #define CC_FATAL (BACKGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

HANDLE g_hConsole = INVALID_HANDLE_VALUE;

void set_console_window(void) {
  g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (g_hConsole == INVALID_HANDLE_VALUE) return;

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
  SetCurrentConsoleFontEx(g_hConsole, FALSE, &FontInfo);
  dwSize.X = 80;
  dwSize.Y = 1024;
  SetConsoleScreenBufferSize(g_hConsole, dwSize);
  SetConsoleMode(g_hConsole, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
  //HMENU hMenu = GetSystemMenu(GetConsoleWindow(), FALSE);
  //EnableMenuItem(hMenu, SC_CLOSE, MF_GRAYED);

  return;
}

#else
  #define CC_NONE "\033[0m"
  #define CC_BLACK "\033[0;30m"
  #define CC_L_BLACK "\033[1;30m"
  #define CC_RED "\033[0;31m"
  #define CC_L_RED "\033[1;31m"
  #define CC_GREEN "\033[0;32m"
  #define CC_L_GREEN "\033[1;32m"
  #define CC_BROWN "\033[0;33m"
  #define CC_YELLOW "\033[1;33m"
  #define CC_BLUE "\033[0;34m"
  #define CC_L_BLUE "\033[1;34m"
  #define CC_PURPLE "\033[0;35m"
  #define CC_L_PURPLE "\033[1;35m"
  #define CC_CYAN "\033[0;36m"
  #define CC_L_CYAN "\033[1;36m"
  #define CC_GRAY "\033[0;37m"
  #define CC_WHITE "\033[1;37m"

  #define CC_BOLD "\033[1m"
  #define CC_UNDERLINE "\033[4m"
  #define CC_BLINK "\033[5m"
  #define CC_REVERSE "\033[7m"
  #define CC_HIDE "\033[8m"
  #define CC_CLEAR "\033[2J"

  #define CC_DBG CC_WHITE
  #define CC_INF CC_GREEN
  #define CC_WRN CC_YELLOW
  #define CC_ERR CC_PURPLE
  #define CC_FATAL CC_RED
#endif

namespace ytlib {

class ConsoleWriter {
 public:
  void Write(const LogData& data);
  void WriteDirectly(const LogData& data);

  bool print_color_ = true;
};

void ConsoleWriter::WriteDirectly(const LogData& data) {
  std::cout << "[" << data.time << "]["
            << static_cast<uint8_t>(data.lvl) << "]["
            << data.thread_id << "]";

  if (data.ctx) {
    for (auto& itr : *(data.ctx))
      std::cout << "[" << itr.first << "=" << itr.second << "]";
  }

  std::cout << data.msg;
}

void ConsoleWriter::Write(const LogData& data) {
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
      SetConsoleTextAttribute(g_hConsole, finditr->second);
      WriteDirectly(data);
      SetConsoleTextAttribute(g_hConsole, CC_DEFAULT);
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

LogWriter GetConsoleWriter(const std::map<std::string, std::string>& cfg) {
  auto pw = std::make_shared<ConsoleWriter>();

  auto finditr = cfg.find("color");
  if (finditr != cfg.end())
    pw->print_color_ = (finditr->second == "true");

#if defined(_WIN32)
  if (pw->print_color_) set_console_window();
#endif

  fprintf(stderr, "init ConsoleWriter, color:%s\n", (pw->print_color_) ? "true" : "false");

  return [pw](const LogData& data) {
    pw->Write(data);
  };
}

}  // namespace ytlib
