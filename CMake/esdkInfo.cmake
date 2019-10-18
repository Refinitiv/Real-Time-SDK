#[================================================================[
This file is the catch-all for defining the attributes for external 
dependencies, as well as interface attributes for locally built and
exported packages/modules  The information contained will define 
attributes for source repositories, values for versioning of  
locally static projects which generate an interface for evaluating 
versioning compatibility.
#]================================================================]
unset(esdk_DEPENDS_LIST)

# esdk Version information
#[====================================================================]
set(esdk_INTERNAL_API_VERSION_MAJOR 3)
set(esdk_INTERNAL_RELEASE_TWEAK G2)
set(esdk_RELEASE_TYPE rrg)

set( librssl_SO_VERSION 9 )
set( librsslVA_SO_VERSION 12 )
set( libema_SO_VERSION 7 )

set(Ansi_VERSION_MAJOR 1)
set(Ansi_VERSION_MINOR 0)
set(Ansi_VERSION_PATCH 0)
set(Ansi_VERSION_TWEAK 44)

# This file is included by other development repos
# and the set to cache is not neccessary
if (DEFINED esdk_VERSION)
	set(esdk_VERSION ${esdk_VERSION} CACHE INTERNAL "")
	set(esdk_VERSION_MAJOR ${esdk_VERSION_MAJOR} CACHE INTERNAL "")
	set(esdk_VERSION_MINOR ${esdk_VERSION_MINOR} CACHE INTERNAL "")
	set(esdk_VERSION_PATCH ${esdk_VERSION_PATCH} CACHE INTERNAL "")
	set(esdk_VERSION_TWEAK ${esdk_VERSION_TWEAK} CACHE INTERNAL "")

	set(esdk_INTERNAL_API_VERSION_MAJOR ${esdk_INTERNAL_API_VERSION_MAJOR} CACHE INTERNAL "")
	set(esdk_INTERNAL_RELEASE_TWEAK ${esdk_INTERNAL_RELEASE_TWEAK} CACHE INTERNAL "")
	set(esdk_RELEASE_TYPE ${esdk_RELEASE_TYPE} CACHE INTERNAL "")

	set(librssl_SO_VERSION ${librssl_SO_VERSION} CACHE INTERNAL "")
	set(librsslVA_SO_VERSION ${librsslVA_SO_VERSION} CACHE INTERNAL "")
	set(libema_SO_VERSION ${libema_SO_VERSION} CACHE INTERNAL "")
endif()

# Build Timestamp
string(TIMESTAMP esdk_timestamp "%a %b %d %H:%M:%S CST")
string(TIMESTAMP esdk_year %Y)

#
#  The following entries are for pre-configuring the external
# projects esdk will add by default.  If the system installed package is
# prefered, it can be used by setting the corresponding flag and version.
# The versions listed below are the minimum version the esdk was verified
# and any other version used are officially not supported by this release.
# However, other version may still work are the user can try newer versions
# at their own risk.  
# The CMAKE option to define for using an installed package :
# (NOTE: not available for lz4)
#        '<package>_USE_SYSTEM_PACKAGE' (the <package>_version field will be
#                                         used as an argument for find_package)
#
# The prefix label for the external projects use the same label as the CMake
# Find<package> module, but in lower case. (e.g. FindZLIB.cmake - ${zlib_url})
#
# Supported external packages:
#       gtest (one exception to this rule, googletest)
#       zlib
#       lz4
#       libxml2
#       cjson 
#       curl 
#
# Each external project has a corresponding cmake module in the esdk/CMake module
# directory.  Each module has the common prefix 'addExternal_'(e.g. addExternal_zlib.cmake).
# At the top of every module, the default package source is defined.  This file may server
# as an override to any of the current package locations.
# An external project entry is defined by the url for the tar-ball, a hash <algo>=ID 
# and the version The supported hash algo's are the CMake supported list:
#    MD5,SHA1,SHA224,SHA256,SHA384,SHA512,SHA3_224,SHA3_256,SHA3_384, or SHA3_512
# 
# The proper syntax is:
#    googletest_url      "https://github.com/abseil/googletest/archive/release-1.8.1.tar.gz"
#    googletest_hash     "MD5=2e6fbeb6a91310a16efe181886c59596"
#    googletest_version  "1.8.1"
#
#
# Elektron-SDK-BinaryPack - default values, for latest, look 
#                           in addExternal_elektron-sdk-binarypack.cmake
#[====================================================================]
# set(elektron-sdk-binarypack_url "https://git.sami.int.thomsonreuters.com/EPD/Elektron-SDK-BinaryPack/repository/Elektron-SDK-BinaryPack.tar.xz")
# set(elektron-sdk-binarypack_hash "MD5=2891965258fec4e2807967866a5aba0a")
# set(elektron-sdk-binarypack_version "1.2.2")
#
# googletest - default values, for latest, look in addExternal_gtest.cmake
#[====================================================================]
# set(googletest_url "https://github.com/abseil/googletest/archive/release-1.8.1.tar.gz")
# set(googletest_hash "MD5=2e6fbeb6a91310a16efe181886c59596")
# set(googletest_version "1.8.1")

#
# zlib - default value, for latest, look in addExternal_zlib.cmake
#[====================================================================]
# set(zlib_url "https://www.zlib.net/zlib-1.2.11.tar.xz")
# set(zlib_hash "SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066")
# set(zlib_version "1.2.11")

#
# lz4 see - default value, for latest, look in addExternal_lz4.cmake
#[====================================================================]
# set(lz4_url "https://github.com/lz4/lz4/archive/v1.8.3.tar.gz"
# set(lz4_hash "MD5=d5ce78f7b1b76002bbfffa6f78a5fc4e")
# set(lz4_version "1.8.3")

#
# libxml2 - default value, for latest, look in addExternal_libxml2.cmake
#[====================================================================]
# set(libxml2_url "ftp://xmlsoft.org/libxml2/libxml2-2.9.9.tar.gz")
# set(libxml2_hash "MD5=c04a5a0a042eaa157e8e8c9eabe76bd6")
# set(libxml2_version "2.9.9")

#
# curl - default value, for latest, look in addExternal_curl.cmake
#[====================================================================]
# set(curl_url "https://github.com/curl/curl/releases/download/curl-7_63_0/curl-7.63.0.tar.xz")
# set(curl_hash "MD5=f43d618cc49c1820d3a2fed31e451d4c")
# set(curl_version "7.63.0")

#
# cjson - default value, for latest, look in addExternal_cjson.cmake
#[====================================================================]
# set(cjson_url "https://github.com/DaveGamble/cJSON/archive/v1.7.10.tar.gz")
# set(curl_hash "MD5=f7ee1a04b7323440f1d7a58ea2c0c197")
# set(curl_version "1.7.10")

# ADD Additional CMake Statements below this point

