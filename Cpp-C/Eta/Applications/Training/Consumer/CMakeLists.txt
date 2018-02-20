# Set prefix for each lib name type
# These are variables are for defining the fields within the .in template file
# for building the suite of training examples
set(appGroup "Consumer")
set(binPrefix "ConsMod")
set(modNumber )
set(sourceFileList )
set(m1 "n AUTO GENERATED" )
set(m2 "NOT" )

# For each example type, create a list of total files, with paths included
#foreach (_item IN LISTS COMP_LIST)
#Get all the files un the current example type
file(GLOB_RECURSE _dirList RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/* )

list(APPEND path_LIST "" )
# Only need one path per example. So first add all path's
# to the example type list, without the file name.  The remove
# the duplicate paths
foreach (_path IN LISTS _dirList)
    string (REGEX MATCH "(Module_.+)(/.*)$" _matchOut ${_path})
    list(APPEND path_LIST ${CMAKE_MATCH_1})
endforeach()

# removing the duplicate path entries
list(REMOVE_DUPLICATES path_LIST)

# For every example path, REGEX out the example number to 
# to construct the example library name: domainPrefixExample#
foreach(_path IN LISTS path_LIST)
    string( REGEX MATCH
            # Module_1a_Connect/
            "Module_([0-9]+[a-zA-Z]*)_([0-9a-zA-Z]+)$"
            _matchOut ${_path}
            )
    if (_matchOut)
        set(modNumber ${CMAKE_MATCH_1})
        # Before writing the CMakeLists example file, get a listing of the
        # source files int the directory
        set(sourceFileList "")
        file (GLOB _pathFiles RELATIVE 
                ${CMAKE_CURRENT_SOURCE_DIR}/${_path} 
                "${_path}/*.c" "${_path}/*.h")
        foreach (_pf IN LISTS _pathFiles)
            string(APPEND sourceFileList "${_pf} ")
        endforeach ()
        
        # Write out the CMakeLists file using the example cmake template
        configure_file(CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/${_path}/CMakeLists.txt @ONLY)
    endif()

	add_subdirectory(${_path})

endforeach()

