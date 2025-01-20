# Helper function to add headers to the global source list
function(nani_add_headers)
  set(SOURCE_FILES "")
  foreach(header IN LISTS ARGV)
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
  endforeach()
  target_sources(${PROJECT_NAME} PUBLIC ${SOURCE_FILES})
endfunction()

# Helper function to add sources to the global source list
function(nani_add_sources)
  set(SOURCE_FILES "")
  foreach(source IN LISTS ARGV)
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
  endforeach()
  target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
endfunction()

