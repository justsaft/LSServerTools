cmake_minimum_required(VERSION 3.20.0)
project(LSServerTools VERSION 0.1.0 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY CXX_STANDARD 14)

add_executable(CommandSender src/cmdsender.cpp)

add_executable(CmdConverter src/converter.cpp)