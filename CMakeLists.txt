cmake_minimum_required(VERSION 3.0)
project(chat)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置可执行文件输出位置
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/./bin)

# 配置头文件的搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/database)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/model)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/redis)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

# 加载子目录
add_subdirectory(src)


