################################################################################

LA_HOME="`sed '1q;d' ~/.libaldaron`"
LA_BIN="`sed '2q;d' ~/.libaldaron`"

help: src/lib-aldaron/ $(FOLDERS) la_config .gitignore
	#
	# Hi!  Welcome to lib-aldaron!
	#	________________________________________________________________
	#	make release --silent	|	Make maximum optimized output.
	#	make test --silent	|	Leave in debug symbols.
	#	make debug --silent	|	Run in GDB.
	#	make android --silent	|	Make for android.
	#	make clean --silent	|	Clean builds.

# Figure out which platform.
include $(shell sed '1q;d' ~/.libaldaron)/compile-scripts/platform.mk

CURDIR=`pwd -P`

# directories
SRC = src
BUILD_OBJ_RELEASE = build/objs
BUILD_OBJ_TEST = build/test
BUILD_OBJ_PROF = build/prof

USERNAME="`sed '1q;d' la_config`"
PACKNAME="`sed '2q;d' la_config`"
PROGNAME="`sed '3q;d' la_config`"

# C & C++ Modules
MODULES = \
	$(subst .c,, $(subst .cpp,, $(shell basename -a \
	$(shell find -L $(SRC)/ -type f -name '*.c') \
	$(shell find -L $(SRC)/ -type f -name '*.cpp') \
)))
HEADERS = $(shell find -L $(SRC)/ -type f -name '*.h')

# Test & Release
OBJS_PROF = $(addprefix $(BUILD_OBJ_PROF)/, $(addsuffix .o,$(MODULES)))
OBJS_TEST = $(addprefix $(BUILD_OBJ_TEST)/, $(addsuffix .o,$(MODULES)))
OBJS_RELEASE = $(addprefix $(BUILD_OBJ_RELEASE)/, $(addsuffix .o,$(MODULES)))

# Special MAKE variable - do not rename.
VPATH = $(shell find -L $(SRC)/ -type d)
# target: init
FOLDERS = build/ src/

################################################################################
.gitignore:
	cp $(LA_HOME)/src/test/.gitignore .gitignore

la_config:
	make la_config2 --silent

la_config2:
	echo
	: > la_config
	echo "Name or company name ( No Uppercase, No spaces ):"
	read -p \ \ \ \ \ \ \ \ \  -r line && echo $$line >> la_config
	echo "Package Name ( No Uppercase, No spaces )"
	read -p \ \ \ \ \ \ \ \ \  -r line && echo $$line >> la_config
	echo "Program Name ( Package Name with Uppercase and Spaces )"
	read -p \ \ \ \ \ \ \ \ \  -r line && echo $$line >> la_config
	echo "Orientation ( Portrait / Landscape / None )"
	read -p \ \ \ \ \ \ \ \ \  -r line && echo $$line >> la_config
	echo
	echo "`cat la_config`"
	echo

src/lib-aldaron/:
	# linking lib-aldaron
	ln -s $(LA_HOME)/src/lib-aldaron src/

-release: build-notify $(FOLDERS) -publish $(OBJS_RELEASE) -link
-test: build-notify $(FOLDERS) -test1 $(OBJS_TEST) -link
-prof: build-notify $(FOLDERS) -test2 $(OBJS_PROF) -link

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
	cp -u $(LA_HOME)/android-build-mods/Android.mk\
	 $(LA_HOME)/src/lib/sdl/android-project/jni/src/
	cp -u $(LA_HOME)/android-build-mods/Android_static.mk\
	 $(LA_HOME)/src/lib/sdl/android-project/jni/src/
	cp -u $(LA_HOME)/android-build-mods/SDL_image-Android.mk\
	 $(LA_HOME)/src/lib/sdl-image/Android.mk
	cp -u $(LA_HOME)/android-build-mods/jconfig.h\
	 $(LA_HOME)/src/lib/sdl-image/external/jpeg-9/jconfig.h
	cp -u $(LA_HOME)/android-build-mods/libzip-Android.mk\
	 $(LA_HOME)/src/lib/libzip/lib/Android.mk
	cp -u $(LA_HOME)/android-build-mods/SDL_config_android.h\
	 $(LA_HOME)/src/lib/sdl/include/SDL_config.h

android: -android-sdl-mods
	cp $(LA_HOME)/android-build-mods/AndroidManifest.xml\
	 $(LA_HOME)/android-build-mods/update/AndroidManifest.xml
	cp $(LA_HOME)/android-build-mods/SDLActivity.java\
	 $(LA_HOME)/android-build-mods/update/src/org/libsdl/app/SDLActivity.java
	# Run Install Script
	export PATH=$$PATH:$(LA_HOME)/src/android-ndk && \
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/tools && \
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/platform-tools && \
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/build-tools/24.0.1 && \
	sh $(LA_HOME)/android-build-mods/androidbuild.sh\
		com.$(USERNAME).$(PACKNAME) $(LA_HOME)/src/C/ $(CURDIR)/$(SRC)/

android-with-ads: -android-sdl-mods
	cp $(LA_HOME)/android-build-mods/AndroidManifest-mm.xml\
	 $(LA_HOME)/android-build-mods/update/AndroidManifest.xml
	cp $(LA_HOME)/android-build-mods/SDLActivity-ad-mm.java\
	 $(LA_HOME)/android-build-mods/update/src/org/libsdl/app/SDLActivity.java
	# Run Install Script
	export PATH=$$PATH:$(LA_HOME)/src/android-ndk && \
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/tools && \
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/platform-tools && \
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/build-tools/24.0.1 && \
	sh $(LA_HOME)/android-build-mods/androidbuild.sh\
		com.$(USERNAME).$(PACKNAME) $(LA_HOME)/src/C/ $(CURDIR)/$(SRC)/

android-deploy:
	scp -P 2222 build/com.$(USERNAME).$(PACKNAME).apk\
	 $(shell echo $(IP)):la_test/com.$(USERNAME).$(PACKNAME).apk

android-debug:
	# Requires a USB cable
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/platform-tools && \
	`which adb` install -r build/com.$(USERNAME).$(PACKNAME).apk && \
	echo "Logcat is starting ( You can open your app now )...." && \
	`which adb` logcat | grep SDL

build-notify:
	# Building program for target=$(PLATFORM)....

clean:
	rm -fr build/bin/ build/objs/ build/test/
	mkdir -p build/bin/ build/objs/ build/test/

clean-jl-lib:
	rm -fr src/lib/jl-lib/

update-jl-lib: clean-jl-lib jl-lib
	# Done!

################################################################################

$(BUILD_OBJ_PROF)/%.o: %.c $(HEADERS)
	echo Compiling Build $<....
	$(CC) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_OBJ_TEST)/%.o: %.c $(HEADERS)
	echo Compiling Test $<....
	$(CC) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)
$(BUILD_OBJ_RELEASE)/%.o: %.c $(HEADERS)
	echo Compiling Release $<....
	$(CC) $(CFLAGS) -o $@ -c $< $(JL_DEBUG)

-init-vars:
	# Build Project
	$(eval CFLAGS_INCLUDES=\
		-I$(LA_HOME)/src/lib/clump/\
		-I$(LA_HOME)/src/lib/libzip/lib\
		-I$(LA_HOME)/src/lib/sdl/include/\
		-I$(LA_HOME)/src/lib/sdl-image/\
		-I$(LA_HOME)/src/lib/sdl-mixer/\
		-I$(LA_HOME)/src/lib/sdl-net/\
		-iquote $(addprefix -I, $(shell find -L src/ -type d ))\
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
-link:
	echo Linking....
	$(CC) $(OBJS) $(LA_HOME)/build/deps.o $(LA_DEPS) \
		-o $(JL_OUT) $(CFLAGS) \
		-lm -lz -ldl -lpthread -lstdc++ -ljpeg \
		$(LINKER_LIBS) $(PLATFORM_CFLAGS) \
		$(GL_VERSION) $(JL_DEBUG)
	echo Done! # [ OpenGL Version = $(GL_VERSION) ]
build/:
	# Generated Files
	mkdir -p build/bin/ # Where the output files are stored
	mkdir -p build/objs/ # Where your program's object files are stored (.o)
	mkdir -p build/test/ # Unoptimized version of build/objs/
src/:
	# Where your program's code files are stored (.c*)
	mkdir -p src/
#end#
