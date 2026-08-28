/**
 * @file conversions.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Helper functions to convert between value types.
 * @date 2026-08-25
 */

#include <algorithm>
#include <cmath>
#include <utility>

namespace Conversions {

/**
 * @brief Converts a number within one range to another range.
 *
 * @remarks Beware integer division!
 *
 * @tparam T Type
 * @param in The number to convert.
 * @param inputRange The {min, max} pair for "in".
 * @param outputRange The {min, max} pair for the return value.
 * @return T The converted number.
 */
template<typename T>
constexpr auto map_range(T in,
                         std::pair<T, T> inputRange,
                         std::pair<T, T> outputRange) -> T
{
    in = std::clamp(in, inputRange.first, inputRange.second);
    return outputRange.first +
           ((outputRange.second - outputRange.first) /
            (inputRange.second - inputRange.first) * (in - inputRange.first));
}

/**
 * @brief Maps internal "control voltage" range (-1, +1) to phase.
 *
 * @tparam T Type
 * @param in CV (-1.0 to 1.0)
 * @return T Phase, from @f$-\pi, +\pi@f$.
 */
template<typename T>
constexpr auto cv_to_phase(T in) -> T
{
    T minIn = -1;
    T maxIn = 1;
    T minOut = -std::numbers::pi_v<T>;
    T maxOut = -minOut;
    return map_range(in, { minIn, maxIn }, { minOut, maxOut });
}

/**
 * @brief Maps phase to internal "control voltage" range (-1, +1).
 *
 * @tparam T Type
 * @param in Phase, from @f$-\pi, +\pi@f$.
 * @return T CV (-1.0 to 1.0)
 */
template<typename T>
constexpr auto phase_to_cv(T in) -> T
{
    T minIn = -std::numbers::pi_v<T>;
    T maxIn = -minIn;
    T minOut = -1;
    T maxOut = 1;
    return map_range(in, { minIn, maxIn }, { minOut, maxOut });
}

/**
 * @brief Maps internal "control voltage" range (-1, +1) to the range of
 * frequencies between MIDI note 0 and 127.
 *
 * @tparam T Type
 * @param in CV (-1.0 to 1.0)
 * @return T Frequency (Hz)
 */
template<typename T>
constexpr auto cv_to_freq(T in) -> T
{
    T minIn = -1;
    T maxIn = 1;
    in = std::clamp(in, minIn, maxIn);
    T minOut = 0;
    T maxOut = 127;
    T midiNoteNumber = map_range(in, { minIn, maxIn }, { minOut, maxOut });
    // From
    // https://en.wikipedia.org/wiki/Musical_note#MIDI
    // NOLINTNEXTLINE(*-magic-numbers)
    return std::pow(2, (midiNoteNumber - 69) / 12) * 440;
}

/**
 * @brief Maps the range of frequencies between MIDI note 0 and 127 to internal
 * "control voltage" range (-1,+1).
 *
 * @tparam T Type
 * @param in Frequency (Hz)
 * @return T CV (-1.0 to 1.0)
 */
template<typename T>
constexpr auto freq_to_cv(T in) -> T
{
    // From
    // https://en.wikipedia.org/wiki/Musical_note#MIDI
    // NOLINTNEXTLINE(*-magic-numbers)
    T midiNoteNumber = T(69) + (T(12) * std::log2(T(in) / T(440)));
    T minIn = 0;   // NOLINT(*magic-numbers)
    T maxIn = 127; // NOLINT(*magic-numbers)
    in = std::clamp(midiNoteNumber, minIn, maxIn);
    T minOut = -1;
    T maxOut = 1;
    return map_range(midiNoteNumber, { minIn, maxIn }, { minOut, maxOut });
}

}