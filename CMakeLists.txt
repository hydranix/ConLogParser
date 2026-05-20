cmake_minimum_required(VERSION 3.28)
project(ConLogParser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(CONLOG_INSTALL          "Generate install rules for ConLogParser" ON)
option(CONLOG_BUILD_EXECUTABLES "Build conlog_parser and dyncmd executables" ON)
option(CONLOG_BUILD_TESTS      "Build parser test suite (ctest)" ON)

include(cmake/CompilerOptions.cmake)

find_package(TBB REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(DynCmd)
add_subdirectory(src)

if(CONLOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CONLOG_INSTALL)
    set(CONLOG_CMAKE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/ConLogParser)

    install(DIRECTORY include/conlog
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT ConLogParserTargets
        FILE ConLogParserTargets.cmake
        NAMESPACE ConLogParser::
        DESTINATION ${CONLOG_CMAKE_INSTALL_DIR}
    )

    configure_package_config_file(
        cmake/ConLogParserConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ConLogParserConfig.cmake
        INSTALL_DESTINATION ${CONLOG_CMAKE_INSTALL_DIR}
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ConLogParserConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ConLogParserConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ConLogParserConfigVersion.cmake
        DESTINATION ${CONLOG_CMAKE_INSTALL_DIR}
    )
endif()
