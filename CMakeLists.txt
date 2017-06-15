project(c2fxp C)

cmake_minimum_required(VERSION 2.8)

add_executable(
	c2enc
	encode.c
	c2enc.c
	fft.c
	)

target_link_libraries(
	c2enc
	LINK_PUBLIC m
	)
