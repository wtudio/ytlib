if command -v doxygen >/dev/null 2>&1; then
  if [ ! -d "$1/html"]; then
    rm -rf "$1/html"
  fi
  doxygen $1/Doxyfile
fi