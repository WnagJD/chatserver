#定义变量SRC_LIST包含需要编译的源文件

aux_source_directory(. SRC_LIST)

aux_source_directory(./db DB_LIST)

aux_source_directory(./model MODEL_LIST)

aux_source_directory(./redis REDIS_LIST)

#生成指定的可行性文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

#指定生成可行性文件需要链接的库文件
target_link_libraries(ChatServer muduo_net muduo_base  mysqlclient hiredis pthread)