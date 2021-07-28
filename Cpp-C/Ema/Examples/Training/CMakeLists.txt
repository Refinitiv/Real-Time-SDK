#[==[
The EMA training examples follow a general naming convention specified as follows:
<#>__<domain>__<main functionality>

- <#>       represents example number
- <domain>  represents message domain type
- <main showcased functionality>
#]==]
# Create a List of the three different types
list(APPEND COMP_LIST Consumer IProvider NiProvider)
# Set prefix for each lib name type
set(ConsumerPrefix "Cons")
set(IProviderPrefix "IProv")
set(NiProviderPrefix "NIProv")
# These are variables are for defining the fields within the .in template file
# for building the suite of training examples
set(exampleNumber )
set(domainPrefix )
set(sourceFileList )
set(m1 "n AUTO GENERATED" )
set(m2 "NOT" )

#   set the REGEX to parse out the path eg Consumer/100_Series/100_MP_Streaming
set(_pathExpression
        "([a-zA-Z]+)/([0-9]+_[a-zA-Z]+)/([0-9]+)_[a-zA-Z]+.*")

# For each example type, create a list of total files, with paths included
foreach (_item IN LISTS COMP_LIST)
    #Get all the files in the current example type
    file(GLOB_RECURSE _dirList RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_item}/*)
    list(APPEND ${_item}_LIST "" )
    # Only need one path per example. So first add all path's
    # to the example type list, without the file name.  Then remove
    # the duplicate paths
    foreach (_path IN LISTS _dirList)
        string (REGEX MATCH "(.+)(/.*)$" _matchOut "${_path}")
        list(APPEND ${_item}_LIST ${CMAKE_MATCH_1})
    endforeach()

    # removing the duplicate path entries
    list(REMOVE_DUPLICATES ${_item}_LIST)

    # For every example path, REGEX out the example number to 
    # to construct the example binary name: domainPrefixExample#
    foreach(_path IN LISTS ${_item}_LIST)
        string( REGEX MATCH
                # NiProvider/100_Series/100_MP_Streaming
                "([a-zA-Z]+)/([0-9]+[_]+[a-zA-Z]+)/([0-9]+)[^a-zA-Z]+([a-zA-Z]+).+$"
                _matchOut "${_path}"
                )
        if (_matchOut)
            set(domainPrefix ${${CMAKE_MATCH_1}Prefix})
            set(exampleNumber ${CMAKE_MATCH_3})

            # Before writing the CMakeLists example file, get a listing of the
            # source files int the directory
            set(sourceFileList "")
            file (GLOB _pathFiles RELATIVE 
                    ${CMAKE_CURRENT_SOURCE_DIR}/${_path} 
                    "${_path}/*.cpp" "${_path}/*.C" "${_path}/*.h" "${_path}/ReadMe.txt")
            foreach (_pf IN LISTS _pathFiles)
                string(APPEND sourceFileList "${_pf} ")
            endforeach ()

            # Write out the CMakeLists file using the example cmake template
            configure_file(CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/${_path}/CMakeLists.txt @ONLY)
        endif()

	add_subdirectory(${_path})

    endforeach()
endforeach()


