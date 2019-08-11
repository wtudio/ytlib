where doxygen
if %ERRORLEVEL% == 0 (
  if exist "%1/html" (
    del "%1/html" /q
  )
  doxygen %1/Doxyfile
)