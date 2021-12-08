#include <filesystem>
#include <string>
#include "ytlib/misc/misc_macro.h"

#include "sqlite3.h"

using namespace std;

static int callback(void *data, int argc, char **argv, char **azColName) {
  DBG_PRINT("data : %s", static_cast<char *>(data));

  for (int ii = 0; ii < argc; ++ii) {
    DBG_PRINT("%s = %s", azColName[ii], argv[ii] ? argv[ii] : "NULL");
  }
  return 0;
}

int32_t main(int32_t argc, char **argv) {
  DBG_PRINT("-------------------start test-------------------");

  sqlite3 *db;
  int ret;
  char *err_msg;
  const char *data = "Callback function called";

  const std::filesystem::path &db_file = std::filesystem::absolute("./test.db");
  DBG_PRINT("db file path is %s", db_file.string().c_str());
  if (std::filesystem::status(db_file).type() == std::filesystem::file_type::regular) {
    std::filesystem::remove(db_file);
  }

  // create db
  DBG_PRINT("-------------------create db-------------------");
  ret = sqlite3_open(db_file.string().c_str(), &db);
  if (ret != SQLITE_OK) {
    DBG_PRINT("sqlite3_open failed, err_msg: %s", sqlite3_errmsg(db));
    return 0;
  } else {
    DBG_PRINT("sqlite3_open successfully");
  }

  // create table
  DBG_PRINT("-------------------create table-------------------");
  std::string sql = R"str(
CREATE TABLE COMPANY(
ID INT PRIMARY KEY     NOT NULL,
NAME           TEXT    NOT NULL,
AGE            INT     NOT NULL,
ADDRESS        CHAR(50),
SALARY         REAL);
)str";

  ret = sqlite3_exec(db, sql.c_str(), callback, (void *)data, &err_msg);
  if (ret != SQLITE_OK) {
    DBG_PRINT("sqlite3_exec error: %s", err_msg);
    sqlite3_free(err_msg);
  } else {
    DBG_PRINT("sqlite3_exec successfully");
  }

  // insert
  DBG_PRINT("-------------------insert-------------------");
  sql = R"str(
INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)
VALUES (1, 'Paul', 32, 'California', 20000.00 );
INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)
VALUES (2, 'Allen', 25, 'Texas', 15000.00 );
INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)
VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );
INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)
VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );
)str";
  ret = sqlite3_exec(db, sql.c_str(), callback, (void *)data, &err_msg);
  if (ret != SQLITE_OK) {
    DBG_PRINT("sqlite3_exec error: %s", err_msg);
    sqlite3_free(err_msg);
  } else {
    DBG_PRINT("sqlite3_exec successfully");
  }

  // select
  DBG_PRINT("-------------------select-------------------");
  sql = "SELECT * from COMPANY";
  ret = sqlite3_exec(db, sql.c_str(), callback, (void *)data, &err_msg);
  if (ret != SQLITE_OK) {
    DBG_PRINT("sqlite3_exec error: %s", err_msg);
    sqlite3_free(err_msg);
  } else {
    DBG_PRINT("sqlite3_exec successfully");
  }

  // update
  DBG_PRINT("-------------------update-------------------");
  sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1;SELECT * from COMPANY";
  ret = sqlite3_exec(db, sql.c_str(), callback, (void *)data, &err_msg);
  if (ret != SQLITE_OK) {
    DBG_PRINT("sqlite3_exec error: %s", err_msg);
    sqlite3_free(err_msg);
  } else {
    DBG_PRINT("sqlite3_exec successfully");
  }

  // delete
  DBG_PRINT("-------------------delete-------------------");
  sql = "DELETE from COMPANY where ID=2;SELECT * from COMPANY";
  ret = sqlite3_exec(db, sql.c_str(), callback, (void *)data, &err_msg);
  if (ret != SQLITE_OK) {
    DBG_PRINT("sqlite3_exec error: %s", err_msg);
    sqlite3_free(err_msg);
  } else {
    DBG_PRINT("sqlite3_exec successfully");
  }

  // close
  DBG_PRINT("-------------------close-------------------");
  sqlite3_close(db);

  DBG_PRINT("********************end test*******************");
  return 0;
}
