add_executable(${PROJECT_NAME} extract.c parser.c utils.c)

find_package(ZLIB)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)
