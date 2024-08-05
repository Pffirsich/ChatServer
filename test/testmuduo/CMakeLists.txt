cmake_minimum_required(VERSION 3.0)
project(main)  #工程的名字 不是可执行文件的名字


#g++ -o server -g muduo_server.cpp xxx.cpp -l/usr/include -L/usr/lib -lmuduo_net -lmuduo_base -lpthread
#配置编译选项  -g
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
#设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件搜索路径  -l/usr/include
# include_directories()
#配置库文件搜索路径  -L/usr/lib
# link_directories()
#设置需要编译的源文件列表  muduo_server.cpp xxx.cpp
set(SRC_LIST ./muduo_server.cpp)
#把.指定路径下的所有源文件名字放入变量名SRC_LIST里面
#aux_source_directory(. SRC_LIST)
#生成可执行文件server，由SRC_LIST变量所定义的源文件编译而来
add_executable(server ${SRC_LIST})
#server目标程序需要连接muduo_net muduo_base pthread这三个库文件
target_link_libraries(server muduo_net muduo_base pthread)