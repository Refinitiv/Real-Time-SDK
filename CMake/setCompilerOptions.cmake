# build flags
if (_compilerOptionsIncluded)
	if ($ENV{RCDEV_DEBUG_ENABLED})
        return()
    endif()
endif()
include(setBinaryEnvironment)
set(_compilerOptionsIncluded TRUE)

if( CMAKE_HOST_UNIX )
	if(NOT ${RCDEV_HOST_SYSTEM_NAME_ABREV})
		set(RCDEV_LINUX_VERSION 7)
	else()
		set(RCDEV_LINUX_VERSION ${RCDEV_HOST_SYSTEM_FLAVOR_REL})
	endif()

	if(${RCDEV_HOST_SYSTEM_BITS} STREQUAL "64")
        set(_compilerBitFlags "-m64 -DCOMPILE_64BITS -fPIC")
    else()
        set(_compilerBitFlags "-m32")
    endif()


	# flags for C
    set( CMAKE_C_FLAGS "${_compilerBitFlags} -D_SVID_SOURCE=1  -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=${RCDEV_LINUX_VERSION} -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE" )
	
	if ( ${CMAKE_BUILD_TYPE} STREQUAL "Optimized" )
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG -O3 -fbuiltin" )
	elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3" )
	endif()

	# flags for C++
    set( CMAKE_CXX_FLAGS "${_compilerBitFlags} -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=${RCDEV_LINUX_VERSION} -Wno-ctor-dtor-privacy -Wno-deprecated -std=c++98 -pthread  -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE" )
	if ( ${CMAKE_BUILD_TYPE} STREQUAL "Optimized" )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -fbuiltin" )
	elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3" )
	endif()

    if( ${RCDEV_HOST_SYSTEM_BITS} STREQUAL "32")
        set(CMAKE_POSITION_INDEPENDENT_CODE 0)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
    endif()
else()
	#Static library debugging symbols setting
	set(RCDEV_DEBUG_TYPE_FLAGS_STATIC "/Z7")
	#Non-static library debugging symbols settings
	set(RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC "/Zi")
	set(RCDEV_TYPE_CHECK_FLAG "/GR")
	
	if(${RCDEV_HOST_SYSTEM_BITS} STREQUAL "64")
		set(RCDEV_COMPILE_BITS "/D COMPILE_64BITS")
	endif()
	
	#Release flags for non-static projects 
	set(RCDEV_FLAGS_NONSTATIC_RELEASE "/GL")
	# flags for C
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS /W3 /Zc:wchar_t- /fp:precise /fp:except- /Gm- /D WIN32 /D _WINDOWS /D _WIN32 /D WIN32_LEAN_AND_MEAN /D _WIN32_WINNT=_WIN32_WINNT_VISTA /D x86_WindowsNT_5X /D _CRT_SECURE_NO_WARNINGS ${RCDEV_COMPILE_BITS} /D _iso_stdcpp_ /errorReport:prompt /WX- /Gd /Zc:forScope /EHsc /nologo /Oi ")
	set(CMAKE_C_FLAGS_DEBUG_MDD "${CMAKE_C_FLAGS} /Gy- /Od /RTC1 /MDd /GF-")
	set(CMAKE_C_FLAGS_RELEASE_MD "${CMAKE_C_FLAGS} /O2 /Ob2 /Ot /MD /Gy /GF /D NDEBUG")
	
	# CXX flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS /W3 /Zc:wchar_t- /fp:precise /fp:except- /Gm- /D WIN32 /D _WINDOWS /D _WIN32 /D WIN32_LEAN_AND_MEAN /D _WIN32_WINNT=_WIN32_WINNT_VISTA /D x86_WindowsNT_5X /D _CRT_SECURE_NO_WARNINGS ${RCDEV_COMPILE_BITS} /D _iso_stdcpp_ /errorReport:prompt /WX- /Gd /Zc:forScope /EHsc /nologo /Oi ")
	set(CMAKE_CXX_FLAGS_DEBUG_MDD "${CMAKE_CXX_FLAGS} /Gy- /Od /RTC1 /MDd /GF-")
	set(CMAKE_CXX_FLAGS_RELEASE_MD "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Ot /MD /Gy /GF /D NDEBUG")
	
	#Linker flags
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG ")
	set( CMAKE_SHARED_LINKER_FLAGS_DEBUG_MDD "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT /INCREMENTAL:NO" )
	set( CMAKE_SHARED_LINKER_FLAGS_RELEASE_MD "${CMAKE_SHARED_LINKER_FLAGS} /LTCG /INCREMENTAL:NO /NODEFAULTLIB:MSVCRTD" )

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
	set( CMAKE_EXE_LINKER_FLAGS_DEBUG_MDD "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT /INCREMENTAL:NO" )
	set( CMAKE_EXE_LINKER_FLAGS_RELEASE_MD "${CMAKE_EXE_LINKER_FLAGS} /LTCG /INCREMENTAL:NO /NODEFAULTLIB:MSVCRTD" )
  
endif()
# end build flags


