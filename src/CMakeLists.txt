add_library(conlog_core STATIC
    EventBus.cpp
    ParserRegistry.cpp
    ProcessGuard.cpp
    FileWatcher.cpp
    LineBuffer.cpp
    Dispatcher.cpp
    KillDeathParser.cpp
    DamageParser.cpp
    NoscopeParser.cpp
    StickyNadesParser.cpp
    SuicideParser.cpp
    PlayerConnectedParser.cpp
    ChatParser.cpp
    PingParser.cpp
    App.cpp
)

add_library(ConLogParser::conlog ALIAS conlog_core)

set_target_properties(conlog_core PROPERTIES
    EXPORT_NAME conlog
)

target_include_directories(conlog_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(conlog_core
    PRIVATE $<BUILD_INTERFACE:conlog_warnings>
    PUBLIC
        TBB::tbb
        Threads::Threads
        dyncmd_core
)

if(CONLOG_BUILD_EXECUTABLES)
    add_executable(conlog_parser main.cpp)
    target_link_libraries(conlog_parser PRIVATE conlog_core)
endif()

if(CONLOG_INSTALL)
    install(TARGETS conlog_core
        EXPORT ConLogParserTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    if(CONLOG_BUILD_EXECUTABLES)
        install(TARGETS conlog_parser
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
endif()
