#include <latren/graphics/postprocessing.h>

#include <cstring>

#define PP_UNIFORM_NAME(name) "postProcessing." #name
#define PP_UPDATE_UNIFORM(shader, uniform) shader.SetUniform(PP_UNIFORM_NAME(uniform), uniform)

void PostProcessing::ApplyUniforms(const Shader& shader) const {
    PP_UPDATE_UNIFORM(shader, kernel.isActive);
    PP_UPDATE_UNIFORM(shader, kernel.blend);
    PP_UPDATE_UNIFORM(shader, kernel.offset);

    PP_UPDATE_UNIFORM(shader, kernel.useKernel3x3);
    PP_UPDATE_UNIFORM(shader, kernel.useKernel5x5);
    PP_UPDATE_UNIFORM(shader, kernel.useKernel7x7);

    PP_UPDATE_UNIFORM(shader, kernel.kernel3x3);
    PP_UPDATE_UNIFORM(shader, kernel.kernel5x5);
    PP_UPDATE_UNIFORM(shader, kernel.kernel7x7);

    PP_UPDATE_UNIFORM(shader, kernel.vignette.isActive);
    PP_UPDATE_UNIFORM(shader, kernel.vignette.size);
    PP_UPDATE_UNIFORM(shader, kernel.vignette.treshold);

    PP_UPDATE_UNIFORM(shader, vignette.isActive);
    PP_UPDATE_UNIFORM(shader, vignette.size);
    PP_UPDATE_UNIFORM(shader, vignette.treshold);
    PP_UPDATE_UNIFORM(shader, vignetteColor);

    PP_UPDATE_UNIFORM(shader, gamma);
    PP_UPDATE_UNIFORM(shader, contrast);
    PP_UPDATE_UNIFORM(shader, brightness);
    PP_UPDATE_UNIFORM(shader, saturation);
}