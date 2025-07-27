#include "utils.h"

float clampf(float value, float min, float max)
{
  if (value < min)
  {
    return min;
  }
  if (value > max)
  {
    return max;
  }

  return value;
}

float passf(float value, float replace, float cut_min, float cut_max)
{
  if (value > cut_min && value < cut_max)
  {
    return replace;
  }

  return value;
}

float mapf(float value, float min_in, float max_in, float min_out, float max_out)
{
  return (value - min_in) / (max_in - min_in) * (max_out - min_out) + min_out;
}
