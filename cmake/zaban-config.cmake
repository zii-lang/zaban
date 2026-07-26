include("cmake/zaban-version.cmake")

set(ZABAN_VERSION_MAJOR ${zaban_version_major})
set(ZABAN_VERSION_MINOR ${zaban_version_minor})
set(ZABAN_VERSION_PATCH ${zaban_version_patch})

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/include/Z/Zaban/Config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Z/Zaban/Config.hpp
)