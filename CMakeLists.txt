cmake_minimum_required(VERSION 3.10)
project(HTTPProxy)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
    src/main.cpp
    src/gui/proxy_gui.cpp    
    src/content_filter/content_filter.cpp       
    src/proxy_server/proxy_server.cpp      
    src/proxy_server/proxy_server_handle.cpp 
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ws2_32)
