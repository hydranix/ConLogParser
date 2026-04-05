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
    App.cpp
)

target_include_directories(conlog_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(conlog_core PUBLIC
    conlog_warnings
    TBB::tbb
    Threads::Threads
)

add_executable(conlog_parser main.cpp)

target_link_libraries(conlog_parser PRIVATE conlog_core)
