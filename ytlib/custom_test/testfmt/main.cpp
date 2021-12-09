#include <string>
#include <vector>
#include "ytlib/misc/misc_macro.h"

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/os.h>
#include <fmt/ranges.h>
#include "fmt/core.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  std::string s = fmt::format("The answer is {:d}.\n", 42);
  DBG_PRINT("s%", s.c_str());

  s = fmt::format("I'd rather be {1} than {0}.\n", "right", "happy");
  DBG_PRINT("s%", s.c_str());

  fmt::print("Default format: {} {}\n", 42s, 100ms);
  fmt::print("strftime-like format: {:%H:%M:%S}\n", 3h + 15min + 30s);

  std::vector<int> v = {1, 2, 3};
  fmt::print("{}\n", v);

  auto out = fmt::output_file("guide.txt");
  out.print("Don't {}", "Panic");

  fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "Hello, {}!\n", "world");
  fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline, "Hello, {}!\n", "world");
  fmt::print(fg(fmt::color::steel_blue) | fmt::emphasis::italic, "Hello, {}!\n", "world");

  DBG_PRINT("********************end test*******************");
  return 0;
}
