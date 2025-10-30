# ACP Examples Build Configuration

# Add example programs
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/acp_client.c")
    add_executable(acp_client acp_client.c)
    target_link_libraries(acp_client acp_static)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mock_serial.c")
    add_executable(mock_serial mock_serial.c)
    target_link_libraries(mock_serial acp_static)
endif()