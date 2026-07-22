/**
 * @file adc.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-07-19
 */

#include <memory_resource>
#include <reusable_synth/software/vertex_interface.hpp>

template<typename DataType, size_t Size, typename... Args>
class AnalogIn : VertexInterface<DataType, Size>
{
    using typename VertexInterface<DataType, Size>::ConnectFunc;


};