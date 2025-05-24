#!/usr/bin/env bash
set -e

# Always use the same compilers that built your Homebrew LLVM
export CC="$(brew --prefix llvm)/bin/clang"
export CXX="$(brew --prefix llvm)/bin/clang++"

rm -rf build
mkdir -p build
cd build

cmake -G Ninja \
  -DLLVM_DIR=$(/opt/homebrew/opt/llvm/bin/llvm-config --cmakedir) \
  -DCMAKE_BUILD_TYPE=Release ..

ninja SimpleConstProp        




