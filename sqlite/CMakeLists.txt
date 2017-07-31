project( sqlite )

#add_definitions()

#sqlite只能静态连接
ADD_LIBRARY( sqlite STATIC 
  sqlite3ext.h
  sqlite3.h
  sqlite3.c
)
