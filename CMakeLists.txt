cmake_minimum_required(VERSION 3.16)
project(BetaGo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(BetaGo src/main.cpp)

target_compile_options(BetaGo PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

target_include_directories(BetaGo PRIVATE ${CMAKE_SOURCE_DIR}/src)

option(BETA_GO_USE_SANITIZERS "Enable AddressSanitizer / UndefinedBehaviorSanitizer" OFF)
if(BETA_GO_USE_SANITIZERS AND NOT MSVC)
  target_compile_options(BetaGo PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(BetaGo PRIVATE -fsanitize=address,undefined)
endif()
