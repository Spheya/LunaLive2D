add_executable (lunalive2d_example "example.cpp")
set_property(TARGET lunalive2d_example PROPERTY CXX_STANDARD 20)
target_link_libraries(lunalive2d_example PUBLIC lunalive2d)

add_custom_command(
	TARGET lunalive2d_example
	POST_BUILD
	COMMAND
		${CMAKE_COMMAND} -E
		copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)