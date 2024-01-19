cmake_minimum_required(VERSION 3.19)

project(relaxisloader LANGUAGES C CXX)

set(SRC_FILES
	relaxisloader.c
	utils.c
)

set(API_HEADERS_DIR relaxisloader/)

set(API_HEADERS_C
	${API_HEADERS_DIR}/relaxisloader.h
)

find_package(PkgConfig REQUIRED)
find_package(Doxygen)
pkg_check_modules(SQL REQUIRED sqlite3)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${API_HEADERS_C})
target_link_libraries(${PROJECT_NAME} ${SQL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ./${API_HEADERS_DIR} ${SQL_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -O2 -march=native -g" LINK_FLAGS "-flto -pthread")
target_compile_definitions(${PROJECT_NAME} PRIVATE _XOPEN_SOURCE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${API_HEADERS_C} DESTINATION include/${PROJECT_NAME})

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.c)
set(LIBS_TEST -L. -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_test ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_test ${LIBS_TEST})
target_include_directories(${PROJECT_NAME}_test PUBLIC ./${API_HEADERS_DIR})
set_target_properties(${PROJECT_NAME}_test PROPERTIES COMPILE_FLAGS "-Wall -O2 -march=native -g" LINK_FLAGS "-flto")
install(TARGETS ${PROJECT_NAME}_test DESTINATION bin)

configure_file(pkgconfig/librelaxisloader.pc.in pkgconfig/librelaxisloader.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/librelaxisloader.pc DESTINATION lib/pkgconfig)

if (DOXYGEN_FOUND)
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/librelaxisloader.doxygen.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/librelaxisloader.doxygen)
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

	set(README_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/readme.md.in)
	set(README_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/readme.md)
	configure_file(${README_IN} ${README_OUT} @ONLY)

	message("Doxygen build started")

	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
