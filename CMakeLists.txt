cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 17)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

project(server VERSION ${PROJECT_VESRION})

#include_directories(/sqlite)

configure_file(version.h.in version.h)

add_executable(server_cli main.cpp)
add_library(lib lib.cpp)
#add_library(server server.cpp)
add_library(server server_async.cpp)

set(SOURCES sqlite/sqlite3.c)
set(HEADERS sqlite/sqlite3.h)
#add_library(sqlite ${SOURCES} ${HEADERS})

add_library(database database.cpp ${SOURCES} ${HEADERS})

#add_library(loger loger.cpp)

set_target_properties(server_cli server lib database PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(lib PRIVATE "${CMAKE_BINARY_DIR}")
#target_include_directories(sqlite PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(database PRIVATE "${CMAKE_BINARY_DIR}")

target_link_libraries(server_cli PRIVATE server)
target_link_libraries(server_cli PRIVATE database)

if (MSVC)
    target_compile_options(server_cli PRIVATE
        /W4
    )
    target_compile_options(server PRIVATE
        /W4
    )
else ()
    target_compile_options(server_cli PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    # target_compile_options(bulk PRIVATE
    #   -Wall -Wextra -pedantic -Werror
    #)
endif()

install(TARGETS server_cli RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)