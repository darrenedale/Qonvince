cmake_minimum_required(VERSION 3.1)

add_library(libqonvince_shared SHARED)
add_library(libqonvince_static STATIC)

set_target_properties(libqonvince_shared libqonvince_static PROPERTIES
	CXX_STANDARD 14
	LIBRARY_OUTPUT_NAME qonvince
	ARCHIVE_OUTPUT_NAME qonvince
	SOVERSION 0.5.0
	SOURCES "src/sharedlibrary.cpp"
#	CXX_CLANG_TIDY clang-tidy-5.0
)

set_target_properties(libqonvince_shared PROPERTIES
	PROJECT_LABEL "LibQonvince (Shared)"
)

set_target_properties(libqonvince_static PROPERTIES
	PROJECT_LABEL "LibQonvince (Static)"
)

# TODO -ldl is only required on unix-like OS
target_link_libraries(libqonvince_shared stdc++fs dl)
target_link_libraries(libqonvince_static stdc++fs dl)
