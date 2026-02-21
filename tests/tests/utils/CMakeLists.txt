target_sources(ReusableSynthTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ring_buffer_test.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/logger_test.cpp 
    # PUBLIC
        # FILE_SET HEADERS 
        # FILES 
)