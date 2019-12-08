cmake_minimum_required (VERSION 2.6)

set(target ${APPLICATION})

file(GLOB srcs "*.cpp")
set_source_files_properties(${srcs} PROPERTIES
    LABELS "${MODULE};Application")

find_package(Threads)

add_executable(server server.cpp)
add_executable(client client.cpp)
set_target_properties(server client PROPERTIES
    OUTPUT_NAME "${MODULE}"
    LABELS "${MODULE};Application")

target_link_libraries(server ${LIBRARY} Threads::Threads)
target_link_libraries(client ${LIBRARY} Threads::Threads)
if (UNIX)
  target_link_libraries(server ${LIBRARY} Threads::Threads)
  target_link_libraries(client ${LIBRARY} Threads::Threads)
endif (UNIX)
