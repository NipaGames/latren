#pragma once

#define _MATH_CONSTANTS_IMPL_GLM

#ifdef _MATH_CONSTANTS_IMPL_GLM
#include <glm/gtc/constants.hpp>
#endif

namespace Math {
    #ifdef _MATH_CONSTANTS_IMPL_GLM
    template <typename T>
    constexpr T PI = glm::pi<T>();
    constexpr float PI_F = PI<float>;
    constexpr double PI_D = PI<double>;

    template <typename T>
    constexpr T E = glm::e<T>();
    constexpr float E_F = E<float>;
    constexpr double E_D = E<double>;
    #endif // _MATH_CONSTANTS_IMPL_GLM

    template <typename T>
    constexpr T PI_2 = T(2) * PI<T>;
    constexpr float PI_2_F = PI_2<float>;
    constexpr double PI_2_D = PI_2<double>;

    template <typename T>
    constexpr T HALF_PI = PI<T> / T(2);
    constexpr float HALF_PI_F = HALF_PI<float>;
    constexpr double HALF_PI_D = HALF_PI<double>;
};
