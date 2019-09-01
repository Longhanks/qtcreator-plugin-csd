#pragma once

#include <type_traits>

namespace CSD {

enum class CaptionButtonStyle : int { custom = 0, win = 1, mac = 2 };

using CaptionButtonStyleType = std::underlying_type_t<CaptionButtonStyle>;

} // namespace CSD
