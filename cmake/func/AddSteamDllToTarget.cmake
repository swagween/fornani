function(nani_add_steam_dll_to_target target_name)
  if (WIN32)
    add_custom_command(
            TARGET ${target_name}
            COMMENT "Copy Steam API DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:${target_name}>/steam_api64.dll
            VERBATIM
    )

    if (MSVC)
      set(subsystem $<IF:$<BOOL:$<CONFIG:Debug>>,CONSOLE,WINDOWS>)
      target_link_options(${target_name} PRIVATE
              /SUBSYSTEM:${subsystem}
              /ENTRY:mainCRTStartup
      )
    endif()
  endif()

  if (FORNANI_DEV_BUILD)
    add_custom_command(
            TARGET ${target_name}
            COMMENT "Write steam_appid.txt"
            PRE_LINK COMMAND ${CMAKE_COMMAND} -E echo ${FORNANI_STEAM_APP_ID} > $<TARGET_FILE_DIR:${target_name}>/steam_appid.txt
            VERBATIM
    )
  endif()
endfunction()
