#[================================================================[
This file is the catch-all for defining the attributes for external 
dependencies, as well as interface attributes for locally built and
exported packages/modules  The information contained will define 
attributes for source repositories, values for versioning of  
locally static projects which generate an interface for evaluating 
versioning compatibility.
#]================================================================]
unset(esdk_DEPENDS_LIST)

#
# esdk Version information
#[====================================================================]
set(INTERNAL_API_VERSION_MAJOR 3)
set(INTERNAL_RELEASE_TWEAK L1)
set(RELEASE_TYPE rrg)

set( librssl_SO_VERSION 8 )
set( librsslVA_SO_VERSION 10 )
set( libema_SO_VERSION 5 )

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
#  In futuer releasesL
#       openssl (coming soon)
#       curl    (coming soon)
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
# Elektron-SDK-BinaryPack
#[====================================================================]
if (DEFINED RCDEV_DL_SOURCE)
    set(_download_site ${RCDEV_DL_SOURCE})
else()
    set(_download_site "https://github.com/thomsonreuters")
endif()
set(Elektron-SDK-BinaryPack_repo "${_download_site}/Elektron-SDK-BinaryPack.git")

if (DEFINED RCDEV_BP_BRANCH)
	set(Elektron-SDK-BinaryPack_tag ${RCDEV_BP_BRANCH})
else()
	set(Elektron-SDK-BinaryPack_tag Elektron-SDK_1.2.2.0.L1)
endif()

#
# googletest - default values in addExternal_gtest.cmake
#[====================================================================]
# set(googletest_url "https://github.com/abseil/googletest/archive/release-1.8.1.tar.gz")
# set(googletest_hash "MD5=2e6fbeb6a91310a16efe181886c59596")
# set(googletest_version "1.8.1")

#
# zlib - default values in addExternal_zlib.cmake
#[====================================================================]
# set(zlib_url "https://github.com/madler/zlib/archive/zlib-1.2.11.tar.xz")
# set(zlib_hash "MD5=85adef240c5f370b308da8c938951a68")
# set(zlib_version "1.2.11")

#
# lz4 see - default values in addExternal_lz4.cmake
#[====================================================================]
# set(lz4_url "https://github.com/lz4/lz4/archive/v1.8.3.tar.gz")
# set(lz4_hash "MD5=d5ce78f7b1b76002bbfffa6f78a5fc4e")
# set(lz4_version "1.8.3")

#
# libxml2 - default values in addExternal_libxml2.cmake
#[====================================================================]
# set(libxml2_url "https://gitlab.gnome.org/GNOME/libxml2/-/archive/v2.9.8/libxml2-v2.9.8.tar.gz")
# set(libxml2_hash "MD5=5e9d56f7f6b564111f9526b2c687256c")
# set(libxml2_version "2.9.8")


