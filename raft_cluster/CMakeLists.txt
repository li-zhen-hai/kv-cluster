include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(LIBS star)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test/raft_cluster)

set(TESTS_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/raft_cluster" TESTS_FILES)

foreach(TESTS_FILE_PATH ${TESTS_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_FILE_NAME ${TESTS_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_FILE_NAME}")
    star_add_executable(${TESTS_FILE_NAME} ${TESTS_FILE_NAME}.cpp star ${LIBS})
endforeach(TESTS_FILE_PATH)