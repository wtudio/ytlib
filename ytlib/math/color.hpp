/**
 * @file color.hpp
 * @brief 颜色
 * @note 颜色相关函数
 * @author WT
 * @date 2022-03-20
 */
#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace ytlib {

inline std::vector<float> rgb2hsb(const std::vector<uint8_t> &rgb) {
  if (rgb.size() != 3)
    throw std::logic_error("invalid rgb vector.");

  std::vector<float> re(3);
  uint8_t rgb_max = *(std::max_element(rgb.begin(), rgb.end()));
  uint8_t rgb_min = *(std::min_element(rgb.begin(), rgb.end()));

  const int &rgb_r = rgb[0], &rgb_g = rgb[1], &rgb_b = rgb[2];
  float &hsb_h = re[0], &hsb_s = re[1], &hsb_b = re[2];

  hsb_b = rgb_max / 255.0f;
  hsb_s = (rgb_max == 0) ? 0.0 : ((rgb_max - rgb_min) / static_cast<float>(rgb_max));

  if (rgb_max == rgb_min) {
    hsb_h = 0.0;
  } else if (rgb_max == rgb_r && rgb_g >= rgb_b) {
    hsb_h = (rgb_g - rgb_b) * 60.0 / (rgb_max - rgb_min) + 0;
  } else if (rgb_max == rgb_r && rgb_g < rgb_b) {
    hsb_h = (rgb_g - rgb_b) * 60.0 / (rgb_max - rgb_min) + 360;
  } else if (rgb_max == rgb_g) {
    hsb_h = (rgb_b - rgb_r) * 60.0 / (rgb_max - rgb_min) + 120;
  } else if (rgb_max == rgb_b) {
    hsb_h = (rgb_r - rgb_g) * 60.0 / (rgb_max - rgb_min) + 240;
  }

  return re;
}

inline std::vector<uint8_t> hsb2rgb(const std::vector<float> &hsb) {
  if (hsb.size() != 3 || hsb[0] < 0.0 || hsb[0] > 360.0 || hsb[1] < 0.0 || hsb[1] > 1.0 || hsb[2] < 0.0 || hsb[2] > 1.0)
    throw std::logic_error("invalid hsb vector.");

  const float &h = hsb[0], &s = hsb[1], &v = hsb[2];
  float r = 0, g = 0, b = 0;
  int i = (static_cast<int>(h / 60) % 6);
  float f = (h / 60) - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  switch (i) {
    case 0:
      r = v;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = v;
      b = p;
      break;
    case 2:
      r = p;
      g = v;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = v;
      break;
    case 4:
      r = t;
      g = p;
      b = v;
      break;
    case 5:
      r = v;
      g = p;
      b = q;
      break;
    default:
      break;
  }
  return std::vector<uint8_t>{static_cast<uint8_t>(r * 255.0 + 0.5),
                              static_cast<uint8_t>(g * 255.0 + 0.5),
                              static_cast<uint8_t>(b * 255.0 + 0.5)};
}
}  // namespace ytlib
