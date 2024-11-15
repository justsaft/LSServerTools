cmake_minimum_required(VERSION 3.5.0)
project(LSServerTools VERSION 0.1.0 LANGUAGES C CXX)

add_executable(CommandSender src/cmdsender.cpp)

add_executable(CmdConverter src/converter.cpp)