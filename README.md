![alt text](https://sites.google.com/site/plopgrizzly/jl-lib/JL_Lib.png)


# INTRODUCTION


JL_LIB is a powerful & easy to use library that uses OpenGL/OpenGLES
with SDL2.  JL_LIB takes care of any compatibility issues there might be between
opengl and opengles and sdl for all supported platforms ( any code written in
this library will run on any supported platform, no extra code writing needed ).

Supported Platforms:
* Linux ( including Raspberry Pi )
* Android

Need help porting:
* Windows PC
* Apple ( Mac )
* IOS
* Others ?

# INSTALL

1. Set up build directories & Download + Build dependencies:
	* For Android compatibility,
		make init-all --silent
	* Without,
		make init-most --silent
2. Compile Library:
	* Build for your computer,
		make build-library --silent
	* Cross compile for ...
		( TODO: TBD )
3. Set Environment Variables
	* In ~/.bashrc
		export JLL_PATH=/home/$(YOUR_NAME_HERE)/bin
		export JLL_HOME=/home/$(YOUR_NAME_HERE)/$(LIB-DIR)/JL_Lib-6

# CLEAN

* Clean all build folders: `make clean-all --silent`
* Clean all dependencies: `make clean-deps --silent`
* Clean all of the build folder: `make clean-build --silent`

# HOW TO BUILD PROJECTS WITH JL_LIB
- Make a makefile that links to compile-scripts/ProjectMakefile.mk
- To compile with maximum optimization use, `make release`
- To compile for testing use, `make test`
- To  run in gdb use, `make debug`
- To build the android apk, sign and install use, `make android`
- To clean builds use, `make clean`

