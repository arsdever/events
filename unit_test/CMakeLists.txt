# Download GTest
include (FetchContent)
FetchContent_Declare (
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevents overriding the parent project's compiler/linker settings
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable (googletest)

add_executable (testapp event.cpp)
target_link_libraries(
    testapp
PUBLIC
    gtest
)

target_include_directories(
    testapp
PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

include (GoogleTest)
#gtest_discover_tests (testapp)