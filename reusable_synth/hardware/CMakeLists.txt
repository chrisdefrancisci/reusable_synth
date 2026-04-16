target_sources(ReusableSynthInterface
    # PRIVATE 
    # put source files here
    PUBLIC
        FILE_SET HEADERS 
        FILES
            circular_dma.hpp 
            interrupt_handler.hpp
            led_interface.hpp
            pin_change.hpp
)