# pissfucks adventure machine
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/cb15f5875813400493337b040ebfa397)](https://app.codacy.com/gh/NipaGames/latren/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
## the experiences will be immaculate
behold the reincarnation of the great opengl-game  
### features:
- [x] basic lighting (phong shading, no shadows)
- [x] resource management system
- [x] weird hybrid ecs + unity esque component mess (making a major rework atm, before it was just the unity-like stuff)
- [x] stage deserialization with whacky pseudo-reflection
- [x] physics (bullet implementation)
- [x] shit runs on my crap school laptop
- [x] primitive ui api
- [ ] audio (although i added openal-soft)
- [ ] editor gui
- [ ] scripting

### third-party dependencies:
*since c++ doesn't have a package manager these are managed as git submodules (so clone recursively)*  
*i'll provide some prebuilt binaries sometime in future, since compiling these takes an eternity*
- [bullet (physics)](https://github.com/bulletphysics/bullet3)
- [glfw (window management)](https://github.com/glfw/glfw)
- [glm (maths)](https://github.com/g-truc/glm)
- [openal-soft (audio)](https://github.com/kcat/openal-soft)
- [assimp (3d model importing)](https://github.com/assimp/assimp)
- [freetype (font importing)](https://gitlab.freedesktop.org/freetype/freetype)
- [fmt (utility; formatting)](https://github.com/fmtlib/fmt)
- [glew-cmake (utility; opengl)](https://github.com/Perlmint/glew-cmake)
- [spdlog (utility; logging)](https://github.com/gabime/spdlog)  
header-only includes *(not submodules)*:  
- [stb_image (texture importing)](https://github.com/nothings/stb/blob/master/stb_image.h)
- [nlohmann/json (json parsing)](https://github.com/nlohmann/json)
- [magic_enum (utility; serialization)](https://github.com/Neargye/magic_enum)