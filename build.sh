rm -rf build/*

cd build

cmake -G Ninja \
  -DLLVM_DIR=$(/opt/homebrew/opt/llvm/bin/llvm-config --cmakedir) \
  -DCMAKE_BUILD_TYPE=Release ..

ninja SimpleConstProp        




