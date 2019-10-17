cmake_minimum_required(VERSION 3.9.5)

project(smack)

set(SMACK_SOURCES
	src/main.cpp
)

add_executable(${PROJECT_NAME} ${SMACK_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VERSION 17)
target_link_libraries(${PROJECT_NAME} ncurses)
