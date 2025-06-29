#pragma once

#include <array>
#include <numeric>
#include <spdlog/spdlog.h>

#include <latren/latren.h>
#include <latren/graphics/shader.h>
#include <latren/defines/mathdefines.h>

class  PostProcessing {
public:
    struct Vignette {
        bool isActive = false;
        float size = .75f;
        float treshold = 0.0f;
    };
    struct Kernel {
        bool isActive = false;
        float blend = 1.0f;
        float offset = 1.0f / 300.0f;

        std::array<float, 9> kernel3x3;
        bool useKernel3x3 = false;
        std::array<float, 25> kernel5x5;
        bool useKernel5x5 = false;
        std::array<float, 49> kernel7x7;
        bool useKernel7x7 = false;

        Vignette vignette;
    };
    Vignette vignette;
    glm::vec3 vignetteColor = glm::vec3(0.0f);
    Kernel kernel;
    float gamma = 1.0f;
    float contrast = 1.0f;
    float brightness = 1.0f;
    float saturation = 1.0f;

    void ApplyUniforms(const Shader&) const;
    template <std::size_t S>
    void ApplyKernel(const std::array<float, S>& k) {
        switch(S) {
            case 9:
                kernel.kernel3x3 = reinterpret_cast<const std::array<float, 9>&>(k);
                kernel.useKernel3x3 = true;
                break;
            case 25:
                kernel.kernel5x5 = reinterpret_cast<const std::array<float, 25>&>(k);
                kernel.useKernel5x5 = true;
                break;
            case 49:
                kernel.kernel7x7 = reinterpret_cast<const std::array<float, 49>&>(k);
                kernel.useKernel7x7 = true;
                break;
            default:
                spdlog::warn("Invalid kernel size!");
                return;
        }
        kernel.isActive = true;
    }
    template <std::size_t S>
    void ApplyKernel(std::array<float, S> k, float factor) {
        for (auto& e : k) {
            e *= factor;
        }
        ApplyKernel(k);
    }
};

namespace Convolution {
    template <std::size_t S>
    constexpr void Normalize(std::array<float, S>& kernel) {
        float sum = std::accumulate(kernel.begin(), kernel.end(), 0.0f, std::plus<float>());
        for (float& f : kernel) {
            f /= sum;
        }
    }
    template <std::size_t S>
    constexpr std::array<float, S * S> Fill(float f) {
        std::array<float, S * S> kernel;
        kernel.fill(f);
        return kernel;
    }
    template <std::size_t S>
    constexpr std::array<float, S * S> BoxBlur() {
        return Fill<S>(1.0f / (S * S));
    }
    // tried decompiling and found out that cmath functions are not actually constexpr for whatever reason
    // so, this isn't actually constexpr
    // anyway, the decompiling was pretty fun :)
    // obviously had no idea what i was doing but managed to make the player move 5 times as fast
    template <std::size_t S>
    constexpr std::array<float, S * S> GaussianBlur(int sigma = S) {
        std::array<float, S * S> kernel;
        float s = 2.0f * (float) (sigma * sigma);
        for (int y = 0; y < S; y++) {
            for (int x = 0; x < S; x++) {
                float distX = std::abs((float) x - (S - 1));
                float distY = std::abs((float) y - (S - 1));
                float val = std::exp(-(distX * distX + distY * distY) / s) / std::sqrt(Math::PI_F * s);
                kernel[y * S + x] = val;
            }
        }
        Normalize(kernel);
        return kernel;
    }
};