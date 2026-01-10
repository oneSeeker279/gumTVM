add_subdirectory(libsmjni)

#xdl source file
file(GLOB XDL_SRC xDL/xdl/src/main/cpp/*.c)
add_library(xdl STATIC ${XDL_SRC})
target_include_directories(xdl PUBLIC xDL/xdl/src/main/cpp/include xDL/xdl/src/main/cpp/)
