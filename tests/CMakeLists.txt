# This CMakeLists.txt is intended to be included through add_subdirectory().
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	message(FATAL_ERROR
		"libzaban_tests must be included as a subdirectory of the root project "
		"using add_subdirectory()."
	)
endif()

project(libzaban_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Zaban library
# ============================================================================

if(ZABAN_BUILD_STATIC)
	set(ZABAN_TESTS_ZABAN_TARGET zaban::static)
elseif(ZABAN_BUILD_SHARED)
	set(ZABAN_TESTS_ZABAN_TARGET zaban::shared)
else()
	message(FATAL_ERROR
		"libzaban_tests requires ZABAN_BUILD_STATIC or ZABAN_BUILD_SHARED "
		"to be enabled."
	)
endif()

if(NOT TARGET ${ZABAN_TESTS_ZABAN_TARGET})
	message(FATAL_ERROR
		"Expected Zaban target '${ZABAN_TESTS_ZABAN_TARGET}' does not exist."
	)
endif()

# ============================================================================
# GoogleTest
# ============================================================================

# First try an explicitly supplied GTEST_DIR.
if(GTEST_DIR)
	message(STATUS "Looking for GoogleTest in GTEST_DIR: ${GTEST_DIR}")

	find_package(GTest CONFIG QUIET
		PATHS "${GTEST_DIR}"
		NO_DEFAULT_PATH
	)
endif()

# Then try the normal system/vcpkg/package-manager locations.
if(NOT GTest_FOUND)
	find_package(GTest CONFIG QUIET)
endif()

# Also allow CMake's FindGTest module to locate system installations
# that don't provide a GTestConfig.cmake.
if(NOT GTest_FOUND)
	find_package(GTest QUIET)
endif()

# ============================================================================
# Fetch GoogleTest if it wasn't found
# ============================================================================

if(NOT GTest_FOUND)
	message(STATUS "GoogleTest not found; fetching GoogleTest with FetchContent.")

	include(FetchContent)

	FetchContent_Declare(
		googletest
		GIT_REPOSITORY
		https://github.com/google/googletest.git
		GIT_TAG
		v1.18.0
		GIT_SHALLOW
		TRUE
	)

	# Don't install GoogleTest when installing this project.
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

	FetchContent_MakeAvailable(googletest)
endif()

# ============================================================================
# Tests
# ============================================================================

add_executable(libzaban_tests
	${CMAKE_CURRENT_SOURCE_DIR}/Zaban/Langs/ZLang/LexerTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GoogleTestTesting.cpp
)

target_link_libraries(libzaban_tests
	PRIVATE
		${ZABAN_TESTS_ZABAN_TARGET}
		GTest::gtest
		GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(libzaban_tests)