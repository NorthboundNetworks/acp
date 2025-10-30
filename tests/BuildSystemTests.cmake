# CMake test configuration for ACP build system tests
# This file adds build system validation as part of the test suite

if(ACP_BUILD_TESTS)
    # Add build validation as a test
    find_program(BASH_PROGRAM bash)
    
    if(BASH_PROGRAM)
        # Quick validation test
        add_test(
            NAME build_validation
            COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_build.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
        set_tests_properties(build_validation PROPERTIES
            TIMEOUT 300
            LABELS "build;validation"
        )
        
        # Comprehensive build test (longer running)
        add_test(
            NAME build_system_comprehensive  
            COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/build_test.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
        set_tests_properties(build_system_comprehensive PROPERTIES
            TIMEOUT 600
            LABELS "build;comprehensive"
        )
        
        # Feature validation test
        if(TARGET acp_static)
            add_test(
                NAME feature_validation
                COMMAND $<TARGET_FILE:acp_feature_test>
            )
            
            set_tests_properties(feature_validation PROPERTIES
                LABELS "feature;validation"
                DEPENDS acp_feature_test
            )
        endif()
        
        message(STATUS "Build system tests added")
    else()
        message(WARNING "Bash not found - build system tests will not be available")
    endif()
endif()