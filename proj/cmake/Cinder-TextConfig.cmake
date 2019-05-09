if( NOT TARGET Cinder-Text )
	get_filename_component( CINDER_TEXT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE )
	include(ExternalProject)

	ExternalProject_Add(
		freetype2a
		PREFIX 3rd_party
		GIT_REPOSITORY git://git.savannah.gnu.org/freetype/freetype2.git
		GIT_SHALLOW 1
		GIT_TAG e92b1d0c5948034f1ebc79994686bc97fec46121
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
	)

	ExternalProject_Add(
		harfbuzz
		PREFIX 3rd_party
		DEPENDS freetype2a
		URL https://github.com/harfbuzz/harfbuzz/releases/download/2.4.0/harfbuzz-2.4.0.tar.bz2
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DHB_HAVE_FREETYPE=On 
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
	)

	ExternalProject_Add(
		freetype2b
		PREFIX 3rd_party
		DEPENDS harfbuzz
		GIT_REPOSITORY git://git.savannah.gnu.org/freetype/freetype2.git
		GIT_SHALLOW 1
		GIT_TAG e92b1d0c5948034f1ebc79994686bc97fec46121
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
	)
	
	list( APPEND CINDER_TEXT_SOURCES
			"${CINDER_TEXT_ROOT}/src/cinder/text/Types.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/TextLayout.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/TextBox.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/SystemFonts.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/Shaper.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/FontManager.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/Font.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/AttributedString.cpp"
			"${CINDER_TEXT_ROOT}/src/cinder/text/gl/TextureRenderer.cpp"
	)
	
	list( APPEND TEXT_LIBRARIES
			"${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/harfbuzz-build/libharfbuzz.a"
			"${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/freetype2b-build/libfreetype.a"
	)
	list( APPEND TEXT_INCLUDES
			"${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/harfbuzz/src"
			"${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/freetype2b/include"
			"${CINDER_TEXT_ROOT}/src"
	)
	add_library( Cinder-Text "${CINDER_TEXT_SOURCES}" )
	add_dependencies( Cinder-Text freetype2b harfbuzz )

	target_link_libraries( Cinder-Text PUBLIC "${TEXT_LIBRARIES}" )
	target_include_directories( Cinder-Text PUBLIC "${TEXT_INCLUDES}" )

	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
			"$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( Cinder-Text PRIVATE cinder )
endif()

