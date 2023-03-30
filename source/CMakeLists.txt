include_directories(${PROJECT_SOURCE_DIR})
# include_directories("/usr/local/include/zookeeper")
# link_libraries("/usr/local/lib/libzookeeper_mt.so")

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic  -std=c++20 -ggdb -Wall -Wno-deprecated -Wno-unused-function -DCMAKE_BUILD_TYPE=Debug")
set(LIB_SRC
        byte_array.cpp
        config.cpp
        fd_manager.cpp
        fiber.cpp
        file_monitor.cpp

        http/http.cpp
        http/http_connection.cpp
        http/http_server.cpp
        http/http_session.cpp
        http/parse.cpp
        http/servlet.cpp
        http/servlets/file_servlet.cpp

        hook.cpp
        io_manager.cpp
        lexical_cast.cpp
        log.cpp

        net/address.cpp
        net/socket.cpp
        net/socket_stream.cpp
        net/tcp_server.cpp
        net/uri.cpp

        rpc/rpc_client.cpp
        rpc/rpc_server.cpp
        rpc/rpc_service_register.cpp
        rpc/rpc_connection_pool.cpp
        rpc/rpc_session.cpp

        raft/raft_server.cpp
        raft/raft_client.cpp

        # CRQA/zab_server.cpp
        # CRQA/zk_client.cpp

        # db/redis.cpp
        db/kv_server.cpp
        db/shared_kv.cpp
        db/kv_client.cpp

        sync/mutex.cpp
        sync/co_condvar.cpp
        sync/distri_lock.cpp

        scheduler.cpp
        stream.cpp
        timer.cpp
        thread.cpp
        util.cpp
        )

set(CMAKE_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../star/proto/)

# protobufmaker(test.proto LIB_SRC ${CMAKE_PROTO_DIR})

set(LIB_LIB pthread yaml-cpp event dl leveldb snappy)

add_library(star STATIC ${LIB_SRC})
target_link_libraries(star ${LIB_LIB})
redefine_file_macro(star)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
