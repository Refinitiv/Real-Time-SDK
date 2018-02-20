
find_package( Doxygen 1.8 )
if (DOXYGEN_FOUND)
        message("Doxygen found")

	# set input and output files
	set(ETA_RSSL_DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/build/Doxyfile.in.ETA)
	set(ETA_RSSL_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.ETA)

	# request to configure the file
	configure_file(${ETA_RSSL_DOXYGEN_IN} ${ETA_RSSL_DOXYGEN_OUT} @ONLY)
	message("Eta rssl Doxygen build configured")

	add_custom_target(doc_eta_rssl_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${ETA_RSSL_DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
		COMMENT "Generating ETA rssl documentation with Doxygen"
		VERBATIM )

	# set input and output files
	set(ETA_VA_DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/build/Doxyfile.in.VA)
	set(ETA_VA_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.VA)

	# request to configure the file
	configure_file(${ETA_VA_DOXYGEN_IN} ${ETA_VA_DOXYGEN_OUT} @ONLY)
	message("Eta VA Doxygen build configured")

	add_custom_target(doc_eta_va_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${ETA_VA_DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
		COMMENT "Generating ETA VA documentation with Doxygen"
		VERBATIM )

else (DOXYGEN_FOUND)
        message("Doxygen not found")
endif()

