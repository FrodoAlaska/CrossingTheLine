if(WIN32)
  set(PROJECT_BUILD_FLAGS 
    "$<$<CONFIG:Debug>:/W3>$<$<CONFIG:Release>:/W0;/O2>"
  )
  set(PROJECT_BUILD_DEFINITIONS 
    "$<$<CONFIG:Debug>:DEBUG; _DEBUG>$<$<CONFIG:Release>:NDEBUG _NDEBUG>"
  )
  set(PROJECT_EXE_TYPE 
    "$<$<CONFIG:Debug>:>$<$<CONFIG:Release>:WIN32>"
  )
elseif(LINUX)
  # Debug builds 
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_FLAGS
      "-Wall"
      "-g"
    )
    set(BUILD_DEFS 
      "DEBUG"
      "_DEBUG"
    )
  # Release builds
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_FLAGS
      "-w"
      "-O3"
      "-s"
    )
    set(BUILD_DEFS 
      "NDEBUG"
      "_NDEBUG"
    )
  endif()
  
  set(PROJECT_BUILD_FLAGS ${BUILD_FLAGS})
  set(PROJECT_BUILD_DEFINITIONS ${BUILD_DEFS})
endif()
