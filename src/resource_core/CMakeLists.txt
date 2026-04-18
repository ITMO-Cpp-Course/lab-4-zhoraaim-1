add_library(resource_core STATIC
        resource_core.cpp
)

target_link_libraries(resource_core PRIVATE project_warnings)

if (TARGET project_sanitizers)
        target_link_libraries(resource_core PRIVATE project_sanitizers)
endif ()

target_include_directories(resource_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
