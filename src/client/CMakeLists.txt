aux_source_directory(. MainList)

add_executable(ChatClient ${MainList})

target_link_libraries(ChatClient pthread)