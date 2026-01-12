cmake_minimum_required(VERSION 3.31)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-macos REQUIRED PATHS node_modules/cmake-macos)
find_package(cmake-windows REQUIRED PATHS node_modules/cmake-windows)

set(name pear-runtime)

project(${name} C)

fetch_package("github:holepunchto/bare@1.26.0")
fetch_package("github:holepunchto/libpath")
fetch_package("github:holepunchto/librlimit")

bare_target(target)

if(target MATCHES "win32")
  add_compile_options(/MT$<$<CONFIG:Debug>:d>)
endif()

add_library(${name}_obj OBJECT)

set_target_properties(
  ${name}_obj
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  ${name}_obj
  PRIVATE
    src/main.c
)

target_include_directories(
  ${name}_obj
  PUBLIC
    $<TARGET_PROPERTY:bare,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
  ${name}_obj
  PUBLIC
    path
    rlimit
)

add_executable(${name}_bin)

set_target_properties(
  ${name}_bin
  PROPERTIES
  ENABLE_EXPORTS ON
  OUTPUT_NAME ${name}
)

target_link_libraries(
  ${name}_bin
  PUBLIC
    ${name}_obj
  PRIVATE
    path_static
    rlimit_static
    $<LINK_LIBRARY:WHOLE_ARCHIVE,bare_static>
)

if(target MATCHES "darwin")
  code_sign_macos(
    ${name}_signature
    TARGET ${name}_bin
    ENTITLEMENTS "entitlements.mac.plist"
    IDENTITY "09590CDE64938DA62FA174A68C240D346B05EF49"
  )
elseif(target MATCHES "win32")
  code_sign_windows(
    ${name}_signature
    TARGET ${name}_bin
    THUMBPRINT "E890AF027C9C83F7427BCEEBE7AC533579DFE549"
  )
endif()
