
include(CMakeDependentOption)

# The default value of BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES is ON 
#      if RCDEV_GSG_PACKAGE is TRUE(ON) 
#         ELSE default value of BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES is OFF
CMAKE_DEPENDENT_OPTION(BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES  
                                "Build applications with prebuilt Eta and Ema libraries" ON
                                "RCDEV_GSG_PACKAGE" OFF)

option(BUILD_ETA_APPLICATIONS   "Build the Eta project applications" ON)

# The default value of BUILD_ETA_PERFTOOLS is ON 
#      if BUILD_ETA_APPLICATIONS is TRUE(ON) 
#         ELSE default value of BUILD_ETA_PERFTOOLS is OFF
CMAKE_DEPENDENT_OPTION(BUILD_ETA_PERFTOOLS  
                                "Build the Eta project performance tools" ON
                                "BUILD_ETA_APPLICATIONS" OFF)

# The default value of BUILD_ETA_EXAMPLES is ON 
#      if BUILD_ETA_APPLICATIONS is TRUE(ON) 
#         ELSE default value of BUILD_ETA_EXAMPLES is OFF
CMAKE_DEPENDENT_OPTION(BUILD_ETA_EXAMPLES   
                                "Build the Eta project examples" ON
                                "BUILD_ETA_APPLICATIONS" OFF )

# The default value of BUILD_ETA_TRAINING is ON 
#      if BUILD_ETA_APPLICATIONS is TRUE(ON) 
#         ELSE default value of BUILD_ETA_TRAINING is OFF
CMAKE_DEPENDENT_OPTION(BUILD_ETA_TRAINING   
                                "Build the Eta project training applications" ON
                                "BUILD_ETA_APPLICATIONS" OFF )
                 
# The default value of BUILD_32_BIT_ETA is OFF 
#      if this is a not a 32-bit build 
#         ELSE default value of BUILD_32_BIT_ETA is ON
CMAKE_DEPENDENT_OPTION(BUILD_32_BIT_ETA			"Build the ETA project as 32-bit" OFF
						"NOT ${RCDEV_HOST_SYSTEM_BITS} EQUAL 32" ON)
# The default value of BUILD_ELEKTRON-SDK-BINARYPACK is ON 
#      if this is a not a 32-bit build 
#         ELSE default value of BUILD_ELEKTRON-SDK-BINARYPACK is OFF
CMAKE_DEPENDENT_OPTION(BUILD_ELEKTRON-SDK-BINARYPACK "Build the Elektron-SDK-BinaryPack Distribution" ON
						"NOT BUILD_32_BIT_ETA" OFF)

mark_as_advanced(BUILD_32_BIT_ETA
				 BUILD_ELEKTRON-SDK-BINARYPACK
				)

# The default value of BUILD_EMA_LIBRARY is ON 
#      if BUILD_32_BIT_ETA is FALSE(OFF) 
#         ELSE default value of BUILD_EMA_LIBRARY is OFF
CMAKE_DEPENDENT_OPTION(BUILD_EMA_LIBRARY        
							"Build the Ema library project" ON
							"NOT BUILD_32_BIT_ETA" OFF)

# The default value of BUILD_EMA_EXAMPLES is ON 
#      if BUILD_EMA_LIBRARY is TRUE(ON) 
#         ELSE default value of BUILD_EMA_EXAMPLES is OFF
CMAKE_DEPENDENT_OPTION(BUILD_EMA_EXAMPLES   
							"Build the Ema project examples" ON
							"BUILD_EMA_LIBRARY;NOT BUILD_32_BIT_ETA" OFF)

# The default value of BUILD_EMA_PERFTOOLS is ON 
#      if BUILD_EMA_LIBRARY is TRUE(ON) 
#         ELSE default value of BUILD_EMA_PERFTOOLS is OFF
CMAKE_DEPENDENT_OPTION(BUILD_EMA_PERFTOOLS   
                                "Build the Ema project performance tools" ON
                                "BUILD_EMA_EXAMPLES" OFF)

# The default value of BUILD_EMA_TRAINING is ON 
#      if BUILD_EMA_LIBRARY is TRUE(ON) 
#         ELSE default value of BUILD_EMA_TRAINING is OFF
CMAKE_DEPENDENT_OPTION(BUILD_EMA_TRAINING  
                                "Build the Ema Training examples" ON
                                "BUILD_EMA_EXAMPLES" OFF)

option(BUILD_UNIT_TESTS "Build unit tests" ON)

# The default value of BUILD_EMA_XXXXX is ON 
#      if BUILD_EMA_LIBRARY is TRUE(ON) 
#         ELSE default value of BUILD_EMA_XXXXX is OFF
CMAKE_DEPENDENT_OPTION(BUILD_ETA_UNIT_TESTS
                                "Build Eta unit tests" ON
                                "BUILD_UNIT_TESTS"  OFF)

CMAKE_DEPENDENT_OPTION(BUILD_EMA_UNIT_TESTS
                                "Build Ema unit tests" ON
                                "BUILD_UNIT_TESTS;NOT BUILD_32_BIT_ETA"  OFF)

option(BUILD_EMA_DOXYGEN "Build Ema doxygen" OFF)

option(BUILD_ETA_DOXYGEN "Build Eta doxygen" OFF)

