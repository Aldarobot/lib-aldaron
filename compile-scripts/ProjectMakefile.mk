################################################################################

# Figure out which platform.
include $(shell echo $(JLL_HOME))/compile-scripts/platform.mk

CURDIR=`pwd -P`

# directories
SRC = src
SRC_DEPS = libs
BUILD_OBJS = build/objs
BUILD_TEST = build/test
BUILD_DEPS = build/deps

PROGNAME="`sed '6q;d' data.txt`"
PACKNAME="`sed '4q;d' data.txt`"
USERNAME="`sed '2q;d' data.txt`"

# Dependencies
#	$(shell find $(SRC_DEPS)/ -type f -name '*.cpp')
MODULES_DEPS_CFILES = $(subst .cpp,, \
	$(shell find $(SRC_DEPS)/ -type f -name '*.c'))
ifeq ("$(MODULES_DEPS_CFILES)", " ")
	MODULES_DEPS = 
else
	MODULES_DEPS = $(subst .c,, $(shell basename -a  $(MODULES_DEPS_CFILES)))
endif
OBJS_DEPS = \
	$(addprefix $(BUILD_DEPS)/, $(addsuffix .o,$(MODULES_DEPS)))
HEADERS_DEPS = \
	$(shell find $(SRC_DEPS)/ -type f -name '*.h')

# Program
#	$(shell find $(SRC)/ -type f -name '*.cpp')
MODULES_PRG = \
	$(subst .c,, $(subst .cpp,, $(shell basename -a \
	$(shell find $(SRC)/ -type f -name '*.c') \
)))
HEADERS_PRG = \
	$(shell find $(SRC)/ -type f -name '*.h')

# Test & Release
OBJS_PRG = \
	$(OBJS_DEPS) \
	$(addprefix $(BUILD_TEST)/, $(addsuffix .o,$(MODULES_PRG)))
OBJS_RLS = \
	$(OBJS_DEPS) \
	$(addprefix $(BUILD_OBJS)/, $(addsuffix .o,$(MODULES_PRG)))

# Special MAKE variable - do not rename.
VPATH = \
	$(shell find $(SRC)/ -type d) \
	$(shell find $(SRC_DEPS)/ -type d)
#
LIB = $(shell echo $(JLL_HOME))/build/jl.o
COMPILE = printf "[COMP/PROJ] Compiling $<....\n";$(CC) # -to- $@.
# target: init
FOLDERS = build/ libs/ src/

################################################################################

-release: build-notify $(FOLDERS) -publish $(OBJS_RLS) -build
-test: build-notify $(FOLDERS) -debug $(OBJS_PRG) -build

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
	cp -u $(shell echo $(JLL_HOME))/android-build-mods/SDLActivity.java\
	 $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/android-project/src/org/libsdl/app/SDLActivity.java

android: -android-sdl-mods
	# Run Install Script
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-ndk-r11c && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/tools && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/platform-tools && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/build-tools/23.0.3 && \
	sh $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/build-scripts/androidbuild.sh\
		jlw.$(USERNAME).$(PACKNAME)\
		$(shell echo $(JLL_HOME))/src/C/ $(CURDIR)/$(SRC)/

android-init: -android-sdl-mods
	# Run Build Script.
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-ndk-r11c && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/tools && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/platform-tools && \
	export PATH=$$PATH:$(shell echo $(JLL_HOME))/deps/android-sdk-linux/build-tools/23.0.3 && \
	sh $(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/build-scripts/androidbuild.sh\
		_ $(shell echo $(JLL_HOME))/src/C/ $(CURDIR)/$(SRC)/

init: $(FOLDERS)
	printf "[COMPILE] Done!\n"

build-notify:
	echo Modules: $(MODULES_PRG) $(MODULES_DEPS)
	echo Headers: $(HEADERS_PRG) / $(HEADERS_DEPS)
	echo Folders: N/A #$(VPATH)
	printf "[COMP] Building program for target=$(PLATFORM)....\n"

clean:
	rm -r build/bin/ build/deps build/objs/ build/test/
	mkdir -p build/bin/ build/deps build/objs/ build/test/

################################################################################

$(BUILD)/%.o: %.c $(HEADERS_PRG) $(HEADERS_DEPS)
	$(COMPILE) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_TEST)/%.o: %.c $(HEADERS_PRG) $(HEADERS_DEPS)
	$(COMPILE) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_OBJS)/%.o: %.c $(HEADERS_PRG) $(HEADERS_DEPS)
	$(COMPILE) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_DEPS)/%.o: %.c $(HEADERS_DEPS)
	printf "[COMP/DEPS] Compiling \"$<\" -to- \"$@\"....\n";
	$(CC) -o $@ -c $< -O3 $(CFLAGS)

-init-vars:
	# Build Project
	$(eval INCLUDES_DEPS=\
		$(addprefix -I, $(shell find $(SRC_DEPS)/ -type d)))
	$(eval CFLAGS_INCLUDES=\
		-I$(shell echo $(JLL_HOME))/src/C/include/\
		-I$(shell echo $(JLL_HOME))/src/lib/include/\
		-iquote $(addprefix -I, $(shell find src/ -type d ))\
		$(addprefix -I, $(shell find $(SRC_DEPS)/ -type d)))
	$(eval CFLAGS=$(CFLAGS_INCLUDES) -Wall)

-debug: -init-vars
#	$(eval GL_VERSION=-lGL) ## OpenGL
	$(eval GL_VERSION=-lGLESv2) ## OpenGL ES
	$(eval JL_DEBUG=-pg -g)
	$(eval JL_OUT=build/test.out)
	$(eval OBJS=$(OBJS_PRG))
-publish: -init-vars
#	$(eval GL_VERSION=-lGL) ## OpenGL
	$(eval GL_VERSION=-lGLESv2) ## OpenGL ES
	$(eval JL_DEBUG=-O3 -Werror)
	$(eval JL_OUT=build/bin/$(PACKNAME))
	$(eval OBJS=$(OBJS_RLS))
-build:
	printf "[COMP] Linking ....\n"
	$(CC) $(OBJS) $(LIB) -o $(JL_OUT) $(CFLAGS) \
		-lm -lz -ldl -lpthread -lstdc++ -ljpeg \
		`$(shell echo $(JLL_HOME))/deps/SDL2-2.0.4/sdl2-config --libs` \
		$(LINKER_LIBS) $(PLATFORM_CFLAGS) \
		$(GL_VERSION) $(JL_DEBUG)
	printf "[COMP] Done [ OpenGL Version = $(GL_VERSION) ]!\n"
build/:
	# Generated Files
	mkdir -p build/bin/ # Where the output files are stored
	mkdir -p build/deps/ # Where your project's dependencies are stored (.o)
	mkdir -p build/objs/ # Where your program's object files are stored (.o)
	mkdir -p build/test/ # Unoptimized version of build/objs/
libs/:
	mkdir -p libs/ # Where the dependencies for your project are stored (.c*)
src/:
	# Where your program's code files are stored (.c*)
	mkdir -p src/include/ # Where your program's header files are stored.
	mkdir -p src/media/ # Where your media files are stored.
#end#
