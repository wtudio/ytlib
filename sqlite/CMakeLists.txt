#---------------------------------------------------------------------
project( sqlite )

#add_definitions()

add_library( sqlite
  sqlite3ext.h
  sqlite3.h
  sqlite3.c
)
