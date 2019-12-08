set(target ${APPLICATION})

file(GLOB srcs "*.cpp")
set_source_files_properties(${srcs} PROPERTIES
    LABELS "${MODULE};Application")

add_executable(server server.cpp)
add_executable(client client.cpp)
set_target_properties(server client PROPERTIES
    OUTPUT_NAME "${MODULE}"
    LABELS "${MODULE};Application")

target_link_libraries(server ${LIBRARY})
target_link_libraries(client ${LIBRARY})
if (UNIX)
  target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT} pthread)
  target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT} pthread)
endif (UNIX)
