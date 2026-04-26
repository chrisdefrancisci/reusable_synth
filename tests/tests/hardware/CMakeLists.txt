target_sources(ReusableSynthTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/circular_dma_test.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/led_test.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/pin_change_test.cpp 
    # PUBLIC
        # FILE_SET HEADERS 
        # FILES 
)