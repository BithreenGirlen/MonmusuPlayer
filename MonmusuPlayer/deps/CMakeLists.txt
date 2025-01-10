cmake_minimum_required (VERSION 3.8)

set(SFML_VERSION "2.6.2")

set(SPINE_VERSION "3.8")
set(SPINE_RUNTIME_LANGUAGE "spine-cpp")

set(NLOHMANN_JSON_VERSION "3.11.3")

set(DIR_SFML ${CMAKE_CURRENT_LIST_DIR}/SFML-${SFML_VERSION})
set(DIR_SPINE_CPP ${CMAKE_CURRENT_LIST_DIR}/spine-cpp)
set(DIR_NLOHMANN_JSON ${CMAKE_CURRENT_LIST_DIR}/nlohmann)

set(DIR_TEMP ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${DIR_TEMP}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${DIR_TEMP}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${DIR_TEMP}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${DIR_TEMP}/${ARCHIVE_NAME}.zip" DESTINATION "${DIR_TEMP}")
	endif()
endfunction()

# Fetch SFML
if (NOT EXISTS "${DIR_SFML}")
	set(SFML_VC_BUILT "SFML-${SFML_VERSION}-windows-vc17-64-bit")
	download_and_extract("https://www.sfml-dev.org/files/${SFML_VC_BUILT}.zip" ${SFML_VC_BUILT})

	file(COPY "${DIR_TEMP}/SFML-${SFML_VERSION}/include" DESTINATION "${DIR_SFML}")
	file(COPY "${DIR_TEMP}/SFML-${SFML_VERSION}/lib" DESTINATION "${DIR_SFML}")

	set(DEBUG_DLLS "sfml-graphics-d-2.dll" "sfml-system-d-2.dll" "sfml-window-d-2.dll")
	set(RELEASE_DLLS "sfml-graphics-2.dll" "sfml-system-2.dll" "sfml-window-2.dll")
	# Debug dll
	foreach(FILENAME IN LISTS DEBUG_DLLS)
		file(COPY "${DIR_TEMP}/SFML-${SFML_VERSION}/bin/${FILENAME}" DESTINATION "${CMAKE_CURRENT_LIST_DIR}/../../x64/Debug")
	endforeach()

	# Release dll
	foreach(FILENAME IN LISTS RELEASE_DLLS)
		file(COPY "${DIR_TEMP}/SFML-${SFML_VERSION}/bin/${FILENAME}" DESTINATION "${CMAKE_CURRENT_LIST_DIR}/../../x64/Release")
	endforeach()
endif()

# Fetch spine-cpp runtime
if (NOT EXISTS "${DIR_SPINE_CPP}")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/${SPINE_VERSION}.zip")
	set (SPINE_FOLDER_NAME "spine-runtimes-${SPINE_VERSION}")
	download_and_extract("${SPINE_RUNTIME_URL}" "${SPINE_FOLDER_NAME}")

	file(COPY "${DIR_TEMP}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/src" DESTINATION "${DIR_SPINE_CPP}")
	file(COPY "${DIR_TEMP}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/include" DESTINATION "${DIR_SPINE_CPP}")
endif()

# Donwload JSON for Modern C++
if (NOT EXISTS "${DIR_NLOHMANN_JSON}")
	set(SINGLE_HEADER_FILENAME "json.hpp")
	set(NLOHMANN_JSON_URL "https://github.com/nlohmann/json/releases/download/v${NLOHMANN_JSON_VERSION}/${SINGLE_HEADER_FILENAME}")
	file(DOWNLOAD "${NLOHMANN_JSON_URL}" "${DIR_NLOHMANN_JSON}/${SINGLE_HEADER_FILENAME}")
endif()

# Copy setup file
if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../x64/Release/setting.txt")
	file(COPY "${CMAKE_CURRENT_LIST_DIR}/setting.txt" DESTINATION "${CMAKE_CURRENT_LIST_DIR}/../../x64/Release")
	file(COPY "${CMAKE_CURRENT_LIST_DIR}/setting.txt" DESTINATION "${CMAKE_CURRENT_LIST_DIR}/../../x64/Debug")
endif()
