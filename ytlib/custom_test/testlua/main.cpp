#include <filesystem>
#include <fstream>
#include "ytlib/misc/misc_macro.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

using namespace std;

extern "C" int LuaPrint(lua_State* L) {
  const char* s = lua_tostring(L, 1);
  DBG_PRINT("%s", s);
  return 0;
}

// 返回传入参数的个数与和
extern "C" int LuaFoo(lua_State* L) {
  int n = lua_gettop(L); /* 参数的个数 */
  lua_Number sum = 0.0;

  int i;
  for (i = 1; i <= n; i++) {
    if (!lua_isnumber(L, i)) {
      lua_pushliteral(L, "invalid argument");
      lua_error(L);
    }
    sum += lua_tonumber(L, i);
  }
  lua_pushnumber(L, n);   /* 第一个返回值 */
  lua_pushnumber(L, sum); /* 第二个返回值 */
  return 2;               /* 返回值的个数 */
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  lua_register(L, "print", LuaPrint);
  lua_register(L, "foo", LuaFoo);

  const std::filesystem::path& script_file = std::filesystem::absolute("./test.lua");

  std::ofstream ofile(script_file, std::ios::trunc);
  ofile << R"str(
-- testlib
print("test test !!!")

local n, sum = foo(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)

print("n:" .. n .. ", sum:" .. sum)

function testluafun(a, b)
    return (a .. b)
end
)str";
  ofile.close();

  int ret = luaL_dofile(L, script_file.string().c_str());
  if (ret) DBG_PRINT("load lua script ret:%d, msg:%s", ret, lua_tostring(L, -1));

  // 调用lua脚本中的一个函数
  lua_getglobal(L, "testluafun");
  lua_pushstring(L, "aaa");
  lua_pushstring(L, "bbb");
  lua_pcall(L, 2, 1, 0);

  std::string lua_ret = lua_tostring(L, -1);
  DBG_PRINT("testluafun ret:%s", lua_ret.c_str());

  lua_close(L);

  DBG_PRINT("********************end test*******************");
  return 0;
}
