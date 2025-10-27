# CMakeLists.txt for ACP tests

# Test configuration
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Test include directories
include_directories(${CMAKE_SOURCE_DIR})

# Test source files will be added as tasks are completed
set(ACP_TEST_SOURCES
    # Core tests (Phase 2-3)
    # byte_order_test.c            # T064
    # crc16_test.c                 # T060  
    # cobs_test.c                  # T028
    # frame_roundtrip_test.c       # T026
    # stream_decode_test.c         # T027
    
    # Authentication tests (Phase 4)
    # hmac_test.c                  # T034, T056
    # replay_test.c                # T035
    # command_auth_reject_test.c   # T052
    # command_auth_bad_tag_test.c  # T053
    
    # Edge case tests (Final Phase)
    # payload_boundary_test.c      # T048
    # crc_mismatch_test.c          # T049
    # no_heap_check.c             # T058
)

# Function to add a test executable
function(add_acp_test test_name test_source)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_source})
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} acp_static)
        target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR})
        
        # Add to CTest
        add_test(NAME ${test_name} COMMAND ${test_name})
        
        # Set test properties
        set_tests_properties(${test_name} PROPERTIES
            TIMEOUT 30
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        
        message(STATUS "Added test: ${test_name}")
    else()
        message(STATUS "Test source not found (will be created): ${test_source}")
    endif()
endfunction()

# Add tests as they become available
add_acp_test(byte_order_test byte_order_test.c)
add_acp_test(crc16_test crc16_test.c)
add_acp_test(cobs_test cobs_test.c)
add_acp_test(frame_roundtrip_test frame_roundtrip_test.c)
add_acp_test(stream_decode_test stream_decode_test.c)
add_acp_test(hmac_test hmac_test.c)
add_acp_test(replay_test replay_test.c)
add_acp_test(command_auth_reject_test command_auth_reject_test.c)
add_acp_test(command_auth_bad_tag_test command_auth_bad_tag_test.c)
add_acp_test(payload_boundary_test payload_boundary_test.c)
add_acp_test(crc_mismatch_test crc_mismatch_test.c)
add_acp_test(no_heap_check no_heap_check.c)

# Test with stub platform shims
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stubs/acp_platform_stubs.c)
    add_subdirectory(stubs)
endif()

# Custom test target for running all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS ${ACP_TEST_SOURCES}
    COMMENT "Running all ACP tests"
)

# Test coverage target (if gcov available)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_custom_target(coverage
        COMMAND mkdir -p coverage
        COMMAND gcov -o ${CMAKE_BINARY_DIR}/CMakeFiles/acp_static.dir *.c
        COMMAND lcov --capture --directory . --output-file coverage/coverage.info
        COMMAND genhtml coverage/coverage.info --output-directory coverage/html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating test coverage report"
    )
endif()

# Test utilities and common functions
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.c)
    add_library(test_utils STATIC test_utils.c)
    target_include_directories(test_utils PUBLIC ${CMAKE_SOURCE_DIR})
endif()