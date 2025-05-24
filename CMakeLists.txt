cmake_minimum_required(VERSION 3.16)
# Enable both C and C++ because LLVM’s helper macros run C test snippets.
project(SimpleConstProp LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)


list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")


include(AddLLVM)


include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


set(LLVM_ENABLE_RTTI ON CACHE BOOL "Enable RTTI for plugins" FORCE)


add_llvm_pass_plugin(SimpleConstProp SimpleConstProp.cpp)