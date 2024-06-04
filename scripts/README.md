# Latren utility shell scripts
*i haven't tried the windows ones in a while so let's just hope they work, alright?*  
## How to run these
**RUN THESE SO THAT THE CWD IS YOUR PROJECT'S ROOT, NOT THIS FOLDER.** otherwise shit won't work for sure  
  
scripts:
- build: builds the project (a shocker)
- run: runs the project (mind absolutely motherfricking blown)
atm you can specify the build type in the build script but you also have to change the path in run too to run the right build type. by default both are set to release.
  
your project's folder structure should look like this after compiling:
- {cwd}/bin/[build type; release or debug]: where the compiled binaries go
- {cwd}/build: where all the cmake bs goes