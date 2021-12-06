#include <filesystem>
#include <string>
#include "ytlib/misc/misc_macro.h"

#include "leveldb/db.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  leveldb::DB* db;

  leveldb::Options options;
  options.create_if_missing = true;
  const std::filesystem::path& p = std::filesystem::absolute("./testdb");

  leveldb::Status status = leveldb::DB::Open(options, p.string(), &db);
  DBG_PRINT("Open status : %s", status.ok() ? "ok" : status.ToString().c_str());

  std::string key = "testkey";
  std::string val = "testval";

  status = db->Put(leveldb::WriteOptions(), key, val);
  DBG_PRINT("Put status : %s", status.ok() ? "ok" : status.ToString().c_str());

  std::string val_out;
  status = db->Get(leveldb::ReadOptions(), key, &val_out);
  DBG_PRINT("Get status : %s", status.ok() ? "ok" : status.ToString().c_str());
  DBG_PRINT("val_out : %s", val_out.c_str());

  status = db->Delete(leveldb::WriteOptions(), key);
  DBG_PRINT("Delete status : %s", status.ok() ? "ok" : status.ToString().c_str());

  status = db->Get(leveldb::ReadOptions(), key, &val_out);
  DBG_PRINT("Get status : %s", status.ok() ? "ok" : status.ToString().c_str());

  delete db;

  DBG_PRINT("********************end test*******************");
  return 0;
}
