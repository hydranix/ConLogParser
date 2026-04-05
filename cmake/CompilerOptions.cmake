add_library(conlog_warnings INTERFACE)

target_compile_options(conlog_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -fsanitize=address)
    add_link_options(-fsanitize=address)
endif()
