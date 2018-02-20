
find_package( Doxygen 1.8 )
if (DOXYGEN_FOUND)
        message("Doxygen found")

	# set input and output files

	set(EMA_DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/build/Doxyfile.in)
	set(EMA_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	# request to configure the file
	configure_file(${EMA_DOXYGEN_IN} ${EMA_DOXYGEN_OUT} @ONLY)
	message("Ema Doxygen build configured")

	add_custom_target(doc_ema_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${EMA_DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
		COMMENT "Generating EMA documentation with Doxygen"
		VERBATIM )

else (DOXYGEN_FOUND)
        message("Doxygen not found")
endif()

