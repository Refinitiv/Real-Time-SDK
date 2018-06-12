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
set(INTERNAL_RELEASE_TWEAK G1)
set(RELEASE_TYPE gload)

set( librssl_SO_VERSION 7 )
set( librsslVA_SO_VERSION 10 )
set( libema_SO_VERSION 4 )

# Build Timestamp
string(TIMESTAMP esdk_timestamp "%a %b %d %H:%M:%S CST")
string(TIMESTAMP esdk_year %Y)

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
	set(Elektron-SDK-BinaryPack_tag Elektron-SDK_1.2.0.1.L1)
endif()

#
# GoogleTest
#[====================================================================]
set(googletest_repo https://github.com/google/googletest.git)
set(googletest_tag master)


