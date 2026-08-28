target_sources(ReusableSynthInterface
    # PRIVATE 
    # put source files here
    PUBLIC
        FILE_SET HEADERS 
        FILES 
            conversions.hpp
            logger.hpp
            noncopyable.hpp
            ring_buffer.hpp
)