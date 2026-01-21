cmake_minimum_required(VERSION 4.0)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-macos REQUIRED PATHS node_modules/cmake-macos)
find_package(cmake-windows REQUIRED PATHS node_modules/cmake-windows)

project(pear_runtime C)

fetch_package("github:holepunchto/bare@1.26.0")
fetch_package("github:holepunchto/libpath")
fetch_package("github:holepunchto/librlimit")
fetch_package("github:holepunchto/libdynload")

bare_target(target)

add_library(pear_runtime_entry SHARED)

set_target_properties(
  pear_runtime_entry
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME pear-runtime.1.0.0
  PREFIX ""
)

target_sources(
  pear_runtime_entry
  PRIVATE
    src/entry.c
)

target_link_libraries(
  pear_runtime_entry
  PUBLIC
    $<LINK_LIBRARY:WHOLE_ARCHIVE,bare_static>
  PRIVATE
    path_static
    rlimit_static
)

add_executable(pear_runtime)

set_target_properties(
  pear_runtime
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME pear-runtime
)

target_sources(
  pear_runtime
  PRIVATE
    src/main.c
)

target_link_libraries(
  pear_runtime
  PRIVATE
    dynload_static
)

install(
  TARGETS pear_runtime bare_shared
)

if(target MATCHES "darwin")
  # code_sign_macos(
  #   pear_runtime_signature
  #   TARGET pear_runtime
  #   ENTITLEMENTS "entitlements.mac.plist"
  #   IDENTITY "09590CDE64938DA62FA174A68C240D346B05EF49"
  # )
elseif(target MATCHES "win32")
  code_sign_windows(
    pear_runtime_signature
    TARGET pear_runtime
    THUMBPRINT "E890AF027C9C83F7427BCEEBE7AC533579DFE549"
  )
endif()

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
