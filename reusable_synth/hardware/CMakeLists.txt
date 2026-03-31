target_sources(ReusableSynthInterface
    # PRIVATE 
    # put source files here
    PUBLIC
        FILE_SET HEADERS 
        FILES 
            interrupt_handler.hpp
            led_interface.hpp
            pin_change.hpp
)