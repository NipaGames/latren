# pissfucks adventure machine
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/cb15f5875813400493337b040ebfa397)](https://app.codacy.com/gh/NipaGames/latren/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
## the experiences will be immaculate
behold the reincarnation of the great opengl-game  
### features and todo:
- check the glfw thread safety and try to move all the calls to the main thread
- [x] basic lighting (phong shading, no shadows)
- [x] resource management system
  - add support for plain text and custom formats
- [x] weird hybrid ecs + unity esque component mess
- [x] stage deserialization with whacky pseudo-reflection
- [x] physics (bullet implementation)
- [x] shit runs on my crap school laptop
  - trying to keep this running at over 100 fps on my shitbox, 60 absolute minimum. for now runs pretty stable even on debug builds. performance isn't my primary concern but if i ever make a finished game with this i want to play it with the laptop too.
- [x] primitive ui api
  - fix text rendering for the hundreth time (sometimes textures too small)
- [x] audio
  - add 3d audio and audiosource component
- [x] custom .cfg file format mainly for resource management and user configs
  - (it's way more complicated than it has any right to be)
- [ ] editor gui
  - for now way out of scope
- [ ] scripting
  - not relevant yet. lua would be simpler to implement probably but python would be so much cooler. FUTURE ME **DO NOT** TRY TO WRITE YOUR OWN LANGUAGES OR PARSERS

### third-party dependencies:
*since c++ doesn't have a package manager these are managed as git submodules (so clone recursively)*  
*i'll provide some prebuilt binaries sometime in future since compiling these takes an eternity*
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
- [stb_vorbis (audio importing)](https://github.com/nothings/stb/blob/master/stb_vorbis.c)
- [nlohmann/json (json parsing)](https://github.com/nlohmann/json)
- [magic_enum (utility; serialization)](https://github.com/Neargye/magic_enum)