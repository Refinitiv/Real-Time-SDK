
if (BUILD_EMA_TRAINING)
    DEBUG_PRINT(BUILD_EMA_TRAINING)
    add_subdirectory( Training ) 
else()
    message(STATUS 
            "Skipping Ema Training Build: BUILD_EMA_TRAINING:${BUILD_EMA_TRAINING}")
endif()

if (BUILD_EMA_PERFTOOLS)
    DEBUG_PRINT(BUILD_EMA_PERFTOOLS)
    add_subdirectory( PerfTools )
else()
    message(STATUS 
            "Skipping Ema PerfTools Build: BUILD_EMA_PERFTOOLS:${BUILD_EMA_PERFTOOLS}")
endif()

if (BUILD_EMA_UNIT_TESTS)
    DEBUG_PRINT(BUILD_EMA_UNIT_TESTS)
    add_subdirectory( Test/UnitTest )
else()
    message(STATUS 
            "Skipping Ema Unit Tests Build: BUILD_EMA_UNIT_TESTS:${BUILD_EMA_UNIT_TESTS}")
endif()

