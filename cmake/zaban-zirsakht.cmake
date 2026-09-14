include(FetchContent)

message(STATUS "Fetching zirsakht library.")

set(ZIRSAKHT_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZIRSAKHT_BUILD_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
	zirsakht
	GIT_REPOSITORY
	https://github.com/zii-lang/zirsakht.git
	GIT_TAG
	main
)

FetchContent_MakeAvailable(zirsakht)