install(
  TARGETS pear_entry pear_runtime
  LIBRARY
    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/lib"
  RUNTIME
    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/bin"
  COMPONENT fixtures
)
