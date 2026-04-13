# Reusable Synth

A library of generic compenents that can be used in a software synthesizer, with an emphasis on embedded applications for modular synths.

## Description

Unit tested software/firmware for modular synthesizers. The underlying goal is to create a platform for flexibly combining algorithms and connecting them to hardware peripherals.

## Installation

Include in your top-level CMakeLists.txt as follows:
```Cmake
find_library(REUSABLE_SYNTH 
    NAMES ReusableSynthInterface 
    HINTS extern/reusable_synth/build/Debug
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
    ${REUSABLE_SYNTH}
)
```

## Usage

All include paths should be relative to the inner reusable_synth directory.

For example, to include the debounced button interface, use
```Cpp
#include <reusable_synth/middleware/debounced_button.hpp>
```

### hardware/

Common interfaces for hardware components:
* GPIO (Pin change)
* Digital-to-Analog Converters (DACs)
* LEDs
* Interrupt callbacks

### middleware/

Common interfaces for logic built on top of hardware:
* Debounced Button

### software/

Software modules:
* Priority task scheduling

Synthesis algorithms:
* Wavetable oscillator

### utils/

A catch-all for the simple stuff:
* Logger
* Ring buffer
* Noncopyable base class

