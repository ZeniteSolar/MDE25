#ifndef __UTILS_H__
#define __UTILS_H__

/**
 * @brief Clamps a value between a minimum and maximum value
 *
 * @param value The value to clamp
 * @param min The minimum value
 * @param max The maximum value
 * @return The clamped value
 */
float clampf(float value, float min, float max);

/**
 * @brief Passes a value through a range, replacing it with a replacement value if it is within a certain range
 *
 * @param value The value to pass through the range
 * @param replace The value to replace the value with if it is within the range
 * @param cut_min The minimum value of the range
 * @param cut_max The maximum value of the range
 * @return The passed value
 */
float passf(float value, float replace, float cut_min, float cut_max);

/**
 * @brief Maps a value from one range to another
 *
 * @param value The value to map
 * @param min_in The minimum value of the input range
 * @param max_in The maximum value of the input range
 * @param min_out The minimum value of the output range
 * @param max_out The maximum value of the output range
 * @return The mapped value
 */
float mapf(float value, float min_in, float max_in, float min_out, float max_out);

#endif /** !__UTILS_H__ */
