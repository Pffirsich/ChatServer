# 定义了一个SRC_LIST变量 存储该文件夹下所有的源文件
aux_source_directory(. SRC_LIST)

# 由该目录下所有源码文件 生成可执行文件ChatServer
add_executable(ChatClient ${SRC_LIST})

# 指定ChatServer链接时需要依赖的库文件
target_link_libraries(ChatClient pthread)