install(
  TARGETS pear_runtime
  DESTINATION "${CMAKE_CURRENT_LIST_DIR}/bin"
  COMPONENT fixtures
)

install(
  TARGETS pear_runtime_entry
  DESTINATION "${CMAKE_CURRENT_LIST_DIR}/lib"
  COMPONENT fixtures
)
