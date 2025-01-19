# Set source and destination directories
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates")
set(DESTINATION_DIR "${CMAKE_CURRENT_BINARY_DIR}/resource")

# Recursively copy all files from the source to the destination
file(GLOB_RECURSE files_to_copy "${SOURCE_DIR}/*")

foreach(file_path IN LISTS files_to_copy)
    # Compute the relative path to preserve directory structure
    file(RELATIVE_PATH relative_path "${SOURCE_DIR}" "${file_path}")
    set(destination_path "${DESTINATION_DIR}/${relative_path}")

    # Use configure_file to copy each file to its destination
    configure_file("${file_path}" "${destination_path}" COPYONLY)
endforeach()

message(STATUS "Template files copied to ${DESTINATION_DIR}")
