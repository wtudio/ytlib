# testlib
add_library(testlib 
  testlib.h
  testlib.cpp
)

SET_PROPERTY(TARGET testlib PROPERTY FOLDER custom_test)

# --------------------------------------------------
# custom_test
add_executable( custom_test
  main.cpp
)

target_link_libraries(custom_test ${YTLIB_LIBRARY_DEPEND})

SET_PROPERTY(TARGET custom_test PROPERTY FOLDER custom_test)
