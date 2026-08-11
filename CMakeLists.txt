cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR
		"In-source builds are not allowed. Use: cmake --preset x64-debug")
endif()
project(libzaban LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(CheckIncludeFiles)
include(GNUInstallDirs)
include("cmake/zaban-config.cmake")
include("cmake/zaban-version.cmake")

set(libname "zaban")
set(cxxflags "")
set(cxxflags_static "")
set(cxxflags_shared "")
set(libraries "")
set(defines "")
set(library_sources "")

set(install_objdir "${CMAKE_INSTALL_LIBDIR}")
set(install_incdir "${CMAKE_INSTALL_INCLUDEDIR}")

option(ZABAN_BUILD_SHARED "Build shared library." ON)
option(ZABAN_BUILD_STATIC "Build static library." ON)

file(GLOB_RECURSE library_sources
	lib/*.cpp
)

if(ZABAN_BUILD_SHARED)
	list(APPEND build_targets "shared")
endif()

if(ZABAN_BUILD_STATIC)
	list(APPEND build_targets "static")
endif()

message(STATUS "")
message(STATUS "Library Name   : ${libname}")
message(STATUS "CXX Compiler   : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Targets  : ${build_targets}")
message(STATUS "Link Libraries : ${libraries}")
message(STATUS "")

if(ZABAN_BUILD_SHARED)
	add_library(zaban-shared SHARED ${library_sources})
	set_target_properties(zaban-shared
		PROPERTIES
			VERSION ${zaban_version}
			SOVERSION ${zaban_version_major}
			OUTPUT_NAME ${libname}
			ARCHIVE_OUTPUT_NAME zaban-import
	)
	target_compile_definitions(zaban-shared
		PRIVATE
			${defines}
			ZABAN_SHARED_LIB
	)
	target_compile_options(zaban-shared
		PRIVATE
			${cxxflags}
			${cxxflags_shared}
	)
	target_link_libraries(zaban-shared PRIVATE ${libraries})
	target_include_directories(zaban-shared
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
			$<INSTALL_INTERFACE:${install_incdir}>
	)
	install(
		TARGETS zaban-shared
		EXPORT zaban
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

	add_library(zaban::shared ALIAS zaban-shared)
endif()

if(ZABAN_BUILD_STATIC)
	add_library(zaban-static STATIC ${library_sources})
	set_property(TARGET zaban-static PROPERTY OUTPUT_NAME ${libname})
	set_property(TARGET zaban-static PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(zaban-static
		PRIVATE
			${defines}
			ZABAN_STATIC_LIB
	)
	target_compile_options(zaban-static
		PRIVATE
			${cxxflags}
			${cxxflags_static}
	)
	target_link_libraries(zaban-static PRIVATE ${libraries})
	target_include_directories(zaban-static
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
			$<INSTALL_INTERFACE:${install_incdir}>
	)
	install(
		TARGETS zaban-static
		EXPORT zaban
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

	add_library(zaban::static ALIAS zaban-static)
endif()

# Install include directories
install(
	DIRECTORY include
	DESTINATION ${install_incdir}
)
