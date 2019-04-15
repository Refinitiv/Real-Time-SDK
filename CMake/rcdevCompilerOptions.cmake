#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
#]=============================================================================]

# build flags
# NOTES FOR TODO:
# For UNIX
#      PARTIALLY DONE with rcdev_update_initial_cache()!
#	1. Set all compiler options/flags for each build type
#         e.g.  _DEBUG, _OPTIMIZED, _OPTIMIZEDDEBUG
#			 CMAKE_CXX_FLAGS_DEBUG=-g
#			 CMAKE_CXX_FLAGS_DEBUG_INIT= -g
#			 CMAKE_C_FLAGS_DEBUG=-g
#			 CMAKE_C_FLAGS_DEBUG_INIT= -g
#			 CMAKE_EXE_LINKER_FLAGS_DEBUG=
#			 CMAKE_MODULE_LINKER_FLAGS_DEBUG=
#			 CMAKE_SHARED_LINKER_FLAGS_DEBUG=
#			 CMAKE_STATIC_LINKER_FLAGS_DEBUG=
#
#      DONE!
#   2. Loop through the build types (DEBUG OPTIMIZED OPTIMIZEDDEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
#      If the current build type is != CMAKE_BUILD_TYPE clear the CACHE entry
#					unset(vars above CACHE)
#         
#   3. Then for the current CMAKE_BUILD_TYPE set the values to CACHE
#     i.e.
#			set_property(CACHE CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_U} ${CMAKE_CXX_FLAGS_DEBUG}
#
if( UNIX )

	if(${RCDEV_HOST_SYSTEM_BITS} STREQUAL "64")
        set(_compilerBitFlags "-m64 -DCOMPILE_64BITS -fPIC")
    else()
        set(_compilerBitFlags "-m32")
    endif()

	# flags for C
    set( CMAKE_C_FLAGS "${_compilerBitFlags} -D_SVID_SOURCE=1  -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=${RCDEV_HOST_SYSTEM_FLAVOR_REL} -pthread -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE" CACHE STRING "" FORCE )
	
	if ( ${CMAKE_BUILD_TYPE} STREQUAL "Optimized" )
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG -O3 -fbuiltin" CACHE STRING "" FORCE)
	elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3"  CACHE STRING "" FORCE)
	endif()

	if (RCDEV_C_EXTRA_FLAGS)
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RCDEV_C_EXTRA_FLAGS}"  CACHE STRING "" FORCE)
	endif()

	# flags for C++
    set( CMAKE_CXX_FLAGS "${_compilerBitFlags} -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=${RCDEV_HOST_SYSTEM_FLAVOR_REL} -Wno-ctor-dtor-privacy -Wno-deprecated -std=c++98 -pthread  -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE"  CACHE STRING "" FORCE)
	if ( ${CMAKE_BUILD_TYPE} STREQUAL "Optimized" )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -fbuiltin" CACHE STRING ""  FORCE)
	elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3"  CACHE STRING "" FORCE)
	endif()

	if (RCDEV_CXX_EXTRA_FLAGS)
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RCDEV_CXX_EXTRA_FLAGS}"  CACHE STRING "" FORCE)
	endif()

	if(APPLE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE"  CACHE STRING "" FORCE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DARWIN_C_SOURCE"  CACHE STRING "" FORCE)
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
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS /W3 /Zc:wchar_t- /fp:precise /fp:except- /Gm- /D WIN32 /D _WINDOWS /D _WIN32 /D WIN32_LEAN_AND_MEAN /D _WIN32_WINNT=_WIN32_WINNT_VISTA /D x86_WindowsNT_5X /D _CRT_SECURE_NO_WARNINGS ${RCDEV_COMPILE_BITS} /D _iso_stdcpp_ /errorReport:prompt /WX- /Gd /Zc:forScope /EHsc /nologo /Oi " CACHE STRING "" FORCE)
	set(CMAKE_C_FLAGS_DEBUG_MDD "${CMAKE_C_FLAGS} /Gy- /Od /RTC1 /MDd /GF-" CACHE STRING "" FORCE)
	set(CMAKE_C_FLAGS_RELEASE_MD "${CMAKE_C_FLAGS} /O2 /Ob2 /Ot /MD /Gy /GF /D NDEBUG" CACHE STRING "" FORCE)

	if (RCDEV_C_EXTRA_FLAGS)
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RCDEV_C_EXTRA_FLAGS}"  CACHE STRING "" FORCE)
	endif()
	if (RCDEV_C_EXTRA_FLAGS_DEBUG_MDD)
		set ( CMAKE_C_FLAGS_DEBUG_MDD "${CMAKE_C_FLAGS_DEBUG_MDD} ${RCDEV_C_EXTRA_FLAGS_DEBUG_MDD}"  CACHE STRING "" FORCE)
	endif()
	if (RCDEV_C_EXTRA_FLAGS_RELEASE_MD)
		set ( CMAKE_C_FLAGS_RELEASE_MD "${CMAKE_C_FLAGS_RELEASE_MD} ${RCDEV_C_EXTRA_FLAGS_RELEASE_MD}"  CACHE STRING "" FORCE)
	endif()
	
	# CXX flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS /W3 /Zc:wchar_t- /fp:precise /fp:except- /Gm- /D WIN32 /D _WINDOWS /D _WIN32 /D WIN32_LEAN_AND_MEAN /D _WIN32_WINNT=_WIN32_WINNT_VISTA /D x86_WindowsNT_5X /D _CRT_SECURE_NO_WARNINGS ${RCDEV_COMPILE_BITS} /D _iso_stdcpp_ /errorReport:prompt /WX- /Gd /Zc:forScope /EHsc /nologo /Oi " CACHE STRING "" FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG_MDD "${CMAKE_CXX_FLAGS} /Gy- /Od /RTC1 /MDd /GF-" CACHE STRING "" FORCE)
	set(CMAKE_CXX_FLAGS_RELEASE_MD "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Ot /MD /Gy /GF /D NDEBUG" CACHE STRING "" FORCE)
	
	if (RCDEV_CXX_EXTRA_FLAGS)
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RCDEV_CXX_EXTRA_FLAGS}"  CACHE STRING "" FORCE)
	endif()
	if (RCDEV_CXX_EXTRA_FLAGS_DEBUG_MDD)
		set ( CMAKE_CXX_FLAGS_DEBUG_MDD "${CMAKE_CXX_FLAGS_DEBUG_MDD} ${RCDEV_CXX_EXTRA_FLAGS_DEBUG_MDD}"  CACHE STRING "" FORCE)
	endif()
	if (RCDEV_CXX_EXTRA_FLAGS_RELEASE_MD)
		set ( CMAKE_CXX_FLAGS_RELEASE_MD "${CMAKE_CXX_FLAGS_RELEASE_MD} ${RCDEV_CXX_EXTRA_FLAGS_RELEASE_MD}"  CACHE STRING "" FORCE)
	endif()
	
	#Linker flags
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG " CACHE STRING "" FORCE)
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_MDD "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT /INCREMENTAL:NO"  CACHE STRING "" FORCE)
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_MD "${CMAKE_SHARED_LINKER_FLAGS} /LTCG /INCREMENTAL:NO /NODEFAULTLIB:MSVCRTD"  CACHE STRING "" FORCE)

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG_MDD "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT /INCREMENTAL:NO"  CACHE STRING "")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE_MD "${CMAKE_EXE_LINKER_FLAGS} /LTCG /INCREMENTAL:NO /NODEFAULTLIB:MSVCRTD"  CACHE STRING "")
  
endif()

unset(_compilerBitFlags)
# end build flags


