################################################################################

# Figure out which platform.
include $(shell echo $(JLL_HOME))/compile-scripts/platform.mk

CURDIR=`pwd -P`

# directories
SRC = src
BUILD_OBJ_RELEASE = build/objs
BUILD_OBJ_TEST = build/test
BUILD_OBJ_PROF = build/prof

PROGNAME="`sed '6q;d' data.txt`"
PACKNAME="`sed '4q;d' data.txt`"
USERNAME="`sed '2q;d' data.txt`"

# C & C++ Modules
MODULES = \
	$(subst .c,, $(subst .cpp,, $(shell basename -a \
	$(shell find $(SRC)/ -type f -name '*.c') \
	$(shell find $(SRC)/ -type f -name '*.cpp') \
)))
HEADERS = $(shell find $(SRC)/ -type f -name '*.h')

# Test & Release
OBJS_PROF = $(addprefix $(BUILD_OBJ_PROF)/, $(addsuffix .o,$(MODULES)))
OBJS_TEST = $(addprefix $(BUILD_OBJ_TEST)/, $(addsuffix .o,$(MODULES)))
OBJS_RELEASE = $(addprefix $(BUILD_OBJ_RELEASE)/, $(addsuffix .o,$(MODULES)))

# Special MAKE variable - do not rename.
VPATH = $(shell find $(SRC)/ -type d)
# target: init
FOLDERS = build/ src/

################################################################################
jl-lib:
	mkdir -p src/lib/jl-lib/
	cp -ur $(shell echo $(JLL_HOME))/src/C/* src/lib/jl-lib/

-release: build-notify $(FOLDERS) -publish $(OBJS_RELEASE) -build
-test: build-notify $(FOLDERS) -test1 $(OBJS_TEST) -build
-prof: build-notify $(FOLDERS) -test2 $(OBJS_PROF) -build

prof: -prof
	./$(JL_OUT)

test: -test
	./$(JL_OUT)

debug: -test
	gdb ./$(JL_OUT)

release: -release
	./$(JL_OUT)

install: -release
	printf "Installing....\n"
	if [ -z "$(JLL_PATH)" ]; then \
		printf "Where to install? ( hint: /bin or $$HOME/bin ) [ Set"\
		 " JLL_PATH ]\n"; \
		read JLL_PATH; \
	fi; \
	printf "Copying files to $$JLL_PATH/....\n"; \
	cp -u --recursive -t $$JLL_PATH/ build/bin/*; \
	printf "Done!\n"

-android-sdl-mods:
	# Apply SDL mods.
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/androidbuild.sh\
	 $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/build-scripts/androidbuild.sh
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/Android.mk\
	 $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/android-project/jni/src/
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/Android_static.mk\
	 $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/android-project/jni/src/
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/SDL_image-Android.mk\
	 $(shell echo $(JLL_HOME))/deps/SDL2_image-2.0.1/Android.mk
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/jconfig.h\
	 $(shell echo $(JLL_HOME))/deps/SDL2_image-2.0.1/external/jpeg-9/jconfig.h
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/libzip-Android.mk\
	 $(shell echo $(JLL_HOME))/deps/libzip-1.1.2/lib/Android.mk

android: -android-sdl-mods
	# Run Install Script
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-ndk-r11c && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/tools && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/platform-tools && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/build-tools/23.0.3 && \
	sh $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/build-scripts/androidbuild.sh\
		jlw.$(USERNAME).$(PACKNAME)\
		$(shell echo $(JLL_HOME))/src/C/ $(CURDIR)/$(SRC)/

init: $(FOLDERS)
	printf "[COMPILE] Done!\n"

build-notify:
	# Building program for target=$(PLATFORM)....

clean:
	rm -r build/bin/ build/deps build/objs/ build/test/
	mkdir -p build/bin/ build/deps build/objs/ build/test/

clean-jl-lib:
	rm -r src/lib/jl-lib/

update-jl-lib: clean-jl-lib jl-lib
	# Done!

################################################################################

$(BUILD_OBJ_PROF)/%.o: %.c $(HEADERS)
	# Compiling Build $<....\n";
	$(CC) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_OBJ_TEST)/%.o: %.c $(HEADERS)
	# Compiling Test $<....\n";
	$(CC) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_OBJ_RELEASE)/%.o: %.c $(HEADERS)
	# Compiling Release $<....\n";
	$(CC) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)

-init-vars:
	# Build Project
	$(eval CFLAGS_INCLUDES=\
		-I$(shell echo $(JLL_HOME))/src/C/include/\
		-I$(shell echo $(JLL_HOME))/src/lib/clump/\
		-I$(shell echo $(JLL_HOME))/src/lib/libzip/lib\
		-I$(shell echo $(JLL_HOME))/src/lib/sdl/include/\
		-I$(shell echo $(JLL_HOME))/src/lib/sdl-image/\
		-I$(shell echo $(JLL_HOME))/src/lib/sdl-mixer/\
		-I$(shell echo $(JLL_HOME))/src/lib/sdl-net/\
		-iquote $(addprefix -I, $(shell find src/ -type d ))\
		$(PLATFORM_INCLUDES))
	$(eval CFLAGS=$(CFLAGS_INCLUDES) -Wall)

-test1: -init-vars
#	$(eval GL_VERSION=-lGL) ## OpenGL
	$(eval GL_VERSION=-lGLESv2) ## OpenGL ES
	$(eval JL_DEBUG=-g -DJL_DEBUG)
	$(eval JL_OUT=build/test.out)
	$(eval OBJS=$(OBJS_TEST))
-test2: -init-vars
#	$(eval GL_VERSION=-lGL) ## OpenGL
	$(eval GL_VERSION=-lGLESv2) ## OpenGL ES
	$(eval JL_DEBUG=-pg -g)
	$(eval JL_OUT=build/test.out)
	$(eval OBJS=$(OBJS_TEST))
-publish: -init-vars
#	$(eval GL_VERSION=-lGL) ## OpenGL
	$(eval GL_VERSION=-lGLESv2) ## OpenGL ES
	$(eval JL_DEBUG=-O3 -Werror)
	$(eval JL_OUT=build/bin/$(PACKNAME))
	$(eval OBJS=$(OBJS_RELEASE))
-build:
	printf "[COMP] Linking ....\n"
	$(CC) $(OBJS) $(shell echo $(JLL_HOME))/build/deps.o \
		-o $(JL_OUT) $(CFLAGS) \
		-lm -lz -ldl -lpthread -lstdc++ -ljpeg \
		`$(shell echo $(JLL_HOME))/src/lib/sdl/sdl2-config --libs` \
		$(LINKER_LIBS) $(PLATFORM_CFLAGS) \
		$(GL_VERSION) $(JL_DEBUG)
	printf "[COMP] Done [ OpenGL Version = $(GL_VERSION) ]!\n"
build/:
	# Generated Files
	mkdir -p build/bin/ # Where the output files are stored
	mkdir -p build/objs/ # Where your program's object files are stored (.o)
	mkdir -p build/test/ # Unoptimized version of build/objs/
src/:
	# Where your program's code files are stored (.c*)
	mkdir -p src/include/ # Where your program's header files are stored.
	mkdir -p src/lib/ # Where the dependencies for your project are stored (.c*)
#end#
