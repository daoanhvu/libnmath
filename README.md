# NMATH

# Overview

## MacOS
Build with clang
```bash
clang++ -std=c++14 -pthread -Iinclude \
  test/testFunction.cpp \
  src/StackUtil.cpp \
  src/nlablexer.cpp \
  src/utf8.cpp \
  src/internal_common.cpp \
  src/logging.cpp \
  -o testFunction
```