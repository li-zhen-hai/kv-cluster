include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(LIBS star)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)

set(TESTS_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/tests" TESTS_FILES)

foreach(TESTS_FILE_PATH ${TESTS_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_FILE_NAME ${TESTS_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_FILE_NAME}")
    star_add_executable(${TESTS_FILE_NAME} ${TESTS_FILE_NAME}.cpp star ${LIBS})
endforeach(TESTS_FILE_PATH)

set(TESTS_RPC_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/tests/rpc" TESTS_RPC_FILES)

foreach(TESTS_RPC_FILE_PATH ${TESTS_RPC_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_RPC_FILE_NAME ${TESTS_RPC_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_RPC_FILE_NAME} ")
    star_add_executable(${TESTS_RPC_FILE_NAME} rpc/${TESTS_RPC_FILE_NAME}.cpp star ${LIBS})
endforeach(TESTS_RPC_FILE_PATH)

set(TESTS_RAFT_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/tests/raft" TESTS_RAFT_FILES)

foreach(TESTS_RAFT_FILE_PATH ${TESTS_RAFT_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_RAFT_FILE_NAME ${TESTS_RAFT_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_RAFT_FILE_NAME} ")
    star_add_executable(${TESTS_RAFT_FILE_NAME} raft/${TESTS_RAFT_FILE_NAME}.cpp star ${LIBS})
endforeach(TESTS_RAFT_FILE_PATH)

# set(TESTS_CRQA_FILES "")
# aux_source_directory("${CMAKE_SOURCE_DIR}/tests/CRQA" TESTS_CRQA_FILES)

# foreach(TESTS_CRQA_FILE_PATH ${TESTS_CRQA_FILES})
#     string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_CRQA_FILE_NAME ${TESTS_CRQA_FILE_PATH})
#     message(STATUS "Generating test target: ${TESTS_CRQA_FILE_NAME} ")
#     star_add_executable(${TESTS_CRQA_FILE_NAME} CRQA/${TESTS_CRQA_FILE_NAME}.cpp star ${LIBS})
# endforeach(TESTS_CRQA_FILE_PATH)

set(TESTS_DB_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/tests/db" TESTS_DB_FILES)

foreach(TESTS_DB_FILE_PATH ${TESTS_DB_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_DB_FILE_NAME ${TESTS_DB_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_DB_FILE_NAME} ")
    star_add_executable(${TESTS_DB_FILE_NAME} db/${TESTS_DB_FILE_NAME}.cpp star ${LIBS})
endforeach(TESTS_DB_FILE_PATH)

# set(TESTS_PROTO_FILES "")
# aux_source_directory("${CMAKE_SOURCE_DIR}/tests/proto" TESTS_PROTO_FILES)

# foreach(TESTS_PROTO_FILE_PATH ${TESTS_PROTO_FILES})
#     string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_PROTO_FILE_NAME ${TESTS_PROTO_FILE_PATH})
#     message(STATUS "Generating test target: ${TESTS_PROTO_FILE_NAME} ")
#     star_add_executable(${TESTS_PROTO_FILE_NAME} proto/${TESTS_PROTO_FILE_NAME}.cpp star ${LIBS})
# endforeach(TESTS_PROTO_FILE_PATH)