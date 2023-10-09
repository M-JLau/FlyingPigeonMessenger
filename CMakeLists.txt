cmake_minimum_required(VERSION 3.25)
project(Chat22)

set(CMAKE_CXX_STANDARD 17)

add_executable(Chat22 main.c broadcast.c create_socket.c filelist.c key_scan_process.c pack_unpack.c tcp_send_file.c udp_process.c userlist.c)
