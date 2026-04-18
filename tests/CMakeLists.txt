include(FetchContent)

FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(catch2)


enable_testing()

add_executable(resource_tests
        tests.cpp
)

target_link_libraries(resource_tests
        PRIVATE
        resource_core
        project_warnings
        Catch2::Catch2WithMain
)

if (TARGET project_sanitizers)
    target_link_libraries(resource_tests PRIVATE project_sanitizers)
endif ()


include(Catch)
catch_discover_tests(resource_tests)


