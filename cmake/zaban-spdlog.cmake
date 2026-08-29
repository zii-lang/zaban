if(ZABAN_USE_SPDLOG)
	find_package(spdlog CONFIG QUIET)

	if(NOT spdlog_FOUND)
		include(FetchContent)

		message(STATUS "spdlog not found, fetching it...")

		FetchContent_Declare(
			spdlog
			GIT_REPOSITORY
			https://github.com/gabime/spdlog.git
			GIT_TAG
			v1.17.0
			GIT_SHALLOW
			TRUE
		)

		FetchContent_MakeAvailable(spdlog)
	endif()
endif()