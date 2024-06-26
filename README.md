# pissfucks adventure machine
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/cb15f5875813400493337b040ebfa397)](https://app.codacy.com/gh/NipaGames/latren/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
## the experiences will be immaculate
behold the reincarnation of the great opengl-game  
### features and todo:
- check the glfw thread safety and try to move all the calls to the main thread
- [x] basic lighting (phong shading, no shadows)
- [x] resource management system
  - add support for ~~plain text~~ (added) and custom formats
- [x] weird hybrid ecs + unity esque component mess
- [x] stage deserialization with whacky pseudo-reflection
- [x] physics (bullet implementation)
- [x] shit runs on my crap school laptop
  - trying to keep this running at over 100 fps on my shitbox, 60 absolute minimum. for now runs pretty stable even on debug builds. performance isn't my primary concern but if i ever make a finished game with this i want to play it with the laptop too.
- [x] primitive-ish ui api
  - text rendering now finally fixed!!! you can render glyphs from atlases and even render textcomponents to intermediate textures for miniscule performance increases and maximum vram usage! i'm pretty proud of the text rendering system now but it's a love-hate-relationship, the text renderer has been by far the most annoying system of the whole engine to get working properly and efficiently.
- [x] audio
  - force 3d stereo audio to mono (spatial stereo audio doesn't play)
  - support more formats (now supporting only ogg vorbis)
- [x] custom .cfg file format mainly for resource management and user configs
  - (it's way more complicated than it has any right to be)
- [ ] editor gui
  - for now way out of scope
- [ ] scripting
  - not relevant yet. lua would be simpler to implement probably but python would be so much cooler. FUTURE ME **DO NOT** TRY TO WRITE YOUR OWN LANGUAGES OR PARSERS

### third-party dependencies:
since c++ doesn't have a universal package manager these are managed as git submodules (so clone recursively)  
dependencies marked with a star (*) are compiled into libs first, others are header-onlies and directly compiled into the binary  
*i'll hopefully provide some prebuilt binaries sometime in future since compiling these takes an eternity*
- [bullet (physics)](https://github.com/bulletphysics/bullet3) *
- [glfw (window management)](https://github.com/glfw/glfw) *
- [glm (maths)](https://github.com/g-truc/glm) *
- [openal-soft (audio)](https://github.com/kcat/openal-soft) *
- [assimp (3d model importing)](https://github.com/assimp/assimp) *
- [freetype (font importing)](https://gitlab.freedesktop.org/freetype/freetype) *
- [fmt (utility; formatting)](https://github.com/fmtlib/fmt) *
- [glew-cmake (utility; opengl)](https://github.com/Perlmint/glew-cmake) *
- [spdlog (utility; logging)](https://github.com/gabime/spdlog) * 
- [stb](https://github.com/nothings/stb), in which:
  - [stb_image (texture importing)](https://github.com/nothings/stb/blob/master/stb_image.h)
  - [stb_vorbis (audio importing)](https://github.com/nothings/stb/blob/master/stb_vorbis.c)
- [nlohmann/json (json parsing)](https://github.com/nlohmann/json)
- [magic_enum (utility; serialization)](https://github.com/Neargye/magic_enum)