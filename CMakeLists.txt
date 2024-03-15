cmake_minimum_required(VERSION 3.25)

set(name pear-runtime)

project(${name} C)

include(bare)

bare_target(target)

add_subdirectory(vendor/bare EXCLUDE_FROM_ALL)

add_subdirectory(vendor/libpath EXCLUDE_FROM_ALL)

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
    $<LINK_LIBRARY:WHOLE_ARCHIVE,bare_static>
)

if(target MATCHES "darwin")
  include(macos)

  code_sign_macos(
    ${name}_signature
    TARGET ${name}_bin
    IDENTITY "09590CDE64938DA62FA174A68C240D346B05EF49"
  )
endif()
