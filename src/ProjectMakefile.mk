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
BUILD_OBJ_RELEASE = build/objs
BUILD_OBJ_TEST = build/test
BUILD_OBJ_PROF = build/prof

USERNAME="`sed '1q;d' la_config`"
PACKNAME="`sed '2q;d' la_config`"
PROGNAME="`sed '3q;d' la_config`"
ORIENT="`sed '4q;d' la_config`"
AD_ID="`sed '5q;d' la_config`"

# C & C++ Modules
MODULES = \
	$(subst .c,, $(subst .cpp,, $(shell basename -a \
	$(shell find -L src/ -type f -name '*.c') \
	$(shell find -L src/ -type f -name '*.cpp') \
)))
HEADERS = $(shell find -L src/ -type f -name '*.h')

# Test & Release
OBJS_PROF = $(addprefix $(BUILD_OBJ_PROF)/, $(addsuffix .o,$(MODULES)))
OBJS_TEST = $(addprefix $(BUILD_OBJ_TEST)/, $(addsuffix .o,$(MODULES)))
OBJS_RELEASE = $(addprefix $(BUILD_OBJ_RELEASE)/, $(addsuffix .o,$(MODULES)))

# Special MAKE variable - do not rename.
VPATH = $(shell find -L src/ -type d)
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
	echo "203888" >> la_config
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
	cp -u $(LA_HOME)/src/android/Android.mk\
	 $(LA_HOME)/src/lib/sdl/android-project/jni/src/
	cp -u $(LA_HOME)/src/android/Android_static.mk\
	 $(LA_HOME)/src/lib/sdl/android-project/jni/src/
	cp -u $(LA_HOME)/src/android/SDL_image-Android.mk\
	 $(LA_HOME)/src/lib/sdl-image/Android.mk
	cp -u $(LA_HOME)/src/android/SDL_mixer-Android.mk\
	 $(LA_HOME)/src/lib/sdl-mixer/Android.mk
	cp -u $(LA_HOME)/src/android/jconfig.h\
	 $(LA_HOME)/src/lib/sdl-image/external/jpeg-9/jconfig.h
	cp -u $(LA_HOME)/src/android/libzip-Android.mk\
	 $(LA_HOME)/src/lib/libzip/lib/Android.mk
	cp -u $(LA_HOME)/src/android/SDL_config_android.h\
	 $(LA_HOME)/src/lib/sdl/include/SDL_config.h

build/android-release-key.keystore:
	keytool -sigalg SHA1withRSA -keyalg RSA -keysize 1024 -genkey -keystore build/android-release-key.keystore -alias daliasle -validity 3650

android-gradle: build/android-release-key.keystore -android-sdl-mods
	# Copy Gradle android project
	cp -ur $(LA_HOME)/src/android-project/ build/
	# build.gradle
	cp $(LA_HOME)/src/android/build.gradle build/android-project/app/build.gradle
	sed -i "s|USERNAME|$(USERNAME)|g" build/android-project/app/build.gradle
	sed -i "s|PACKNAME|$(PACKNAME)|g" build/android-project/app/build.gradle
	# AndroidManifest.xml
	cp $(LA_HOME)/src/android/AndroidManifest.xml build/android-project/app/src/main/AndroidManifest.xml
	sed -i "s|USERNAME|$(USERNAME)|g" build/android-project/app/src/main/AndroidManifest.xml
	sed -i "s|PACKNAME|$(PACKNAME)|g" build/android-project/app/src/main/AndroidManifest.xml
	PROGNAME=$(PROGNAME) && sed -i "s|PROGNAME|$$PROGNAME|g" build/android-project/app/src/main/AndroidManifest.xml
	sed -i "s|ORIENT|$(ORIENT)|g" build/android-project/app/src/main/AndroidManifest.xml
	# ic_launcher.png
	cp resources/icon.png build/android-project/app/src/main/res/drawable/ic_launcher.png
	# SDLActivity.java
	mkdir -p build/android-project/app/src/main/java/com/libaldaron/
	cp $(LA_HOME)/src/android/LibAldaronActivity.java build/android-project/app/src/main/java/com/libaldaron/LibAldaronActivity.java
	sed -i "s|203888|$(AD_ID)|g" build/android-project/app/src/main/java/com/libaldaron/LibAldaronActivity.java
	# SDLActivity.java #2
	mkdir -p build/android-project/app/src/main/java/com/$(USERNAME)/$(PACKNAME)/
	echo "package com.$(USERNAME).$(PACKNAME);" > build/android-project/app/src/main/java/com/$(USERNAME)/$(PACKNAME)/AldaronActivity.java
	echo "import com.libaldaron.LibAldaronActivity;" >> build/android-project/app/src/main/java/com/$(USERNAME)/$(PACKNAME)/AldaronActivity.java
	echo "public class AldaronActivity extends LibAldaronActivity {}" >> build/android-project/app/src/main/java/com/$(USERNAME)/$(PACKNAME)/AldaronActivity.java
	# Link SDL sources
	mkdir -p build/android-project/app/src/main/jni/SDL
	ln -sf $(LA_HOME)/src/lib/sdl/src build/android-project/app/src/main/jni/SDL
	ln -sf $(LA_HOME)/src/lib/sdl/include build/android-project/app/src/main/jni/SDL
	cp -u $(LA_HOME)/src/lib/sdl/Android.mk build/android-project/app/src/main/jni/SDL
	# Link SDL_image sources
	ln -sTf $(LA_HOME)/src/lib/sdl-image/ build/android-project/app/src/main/jni/SDL_image
	# Link SDL_mixer sources
	ln -sTf $(LA_HOME)/src/lib/sdl-mixer/ build/android-project/app/src/main/jni/SDL_mixer
	ln -sTf $(LA_HOME)/src/lib/sdl-mixer/external/smpeg2-2.0.0/ build/android-project/app/src/main/jni/smpeg2
	# Link SDL_net sources
	ln -sTf $(LA_HOME)/src/lib/sdl-net/ build/android-project/app/src/main/jni/SDL_net
	# Link Lib Zip sources
	cp -u $(LA_HOME)/src/lib/libzip/config.h $(LA_HOME)/src/lib/libzip/lib/config.h
	ln -sTf $(LA_HOME)/src/lib/libzip/lib/ build/android-project/app/src/main/jni/libzip
	# Link Clump sources
	ln -sTf $(LA_HOME)/src/lib/clump/ build/android-project/app/src/main/jni/clump
	# Link Android Native App Glue!
	ln -sTf $(LA_HOME)/src/android-ndk/sources/android/native_app_glue/\
	 build/android-project/app/src/main/jni/native_app_glue
	# Link User sources
	cp -u $(LA_HOME)/src/lib/sdl/android-project/jni/src/Android.mk build/android-project/app/src/main/jni/src/
	ln -fs $(CURDIR)/src/ build/android-project/app/src/main/jni/src/
	# NDK
	cd build/android-project/app/src/main/jni/ && $(LA_HOME)/src/android-ndk/ndk-build -j `nproc`
	# Gradle
	cd build/android-project/ && \
#	ANDROID_HOME=$(LA_HOME)/src/android-sdk ./gradlew && \
	ANDROID_HOME=$(LA_HOME)/src/android-sdk ./gradlew assembleRelease
	# Sign
	jarsigner -verbose -tsa http://timestamp.digicert.com -sigalg SHA1withRSA -digestalg SHA1 -keystore $(CURDIR)/build/android-release-key.keystore build/android-project/app/build/outputs/apk/app-release-unsigned.apk daliasle
	# Zipalign & Rename
	rm -f build/com.$(USERNAME).$(PACKNAME).apk
	$(LA_HOME)/src/android-sdk/build-tools/23.0.3/zipalign -v 4 build/android-project/app/build/outputs/apk/app-release-unsigned.apk build/com.$(USERNAME).$(PACKNAME).apk

android-deploy:
	scp -P 2222 build/com.$(USERNAME).$(PACKNAME).apk\
	 $(shell echo $(IP)):la_test/com.$(USERNAME).$(PACKNAME).apk

android-debug:
	# Requires a USB cable
	export PATH=$$PATH:$(LA_HOME)/src/android-sdk/platform-tools && \
	`which adb` install -r build/com.$(USERNAME).$(PACKNAME).apk && \
	echo "Logcat is starting ( You can open your app now )...." && \
	`which adb` logcat | grep Aldaron # grep SDL # grep LA_EVENT # grep $(PACKNAME)

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
		-iquote\
		-Isrc/\
		$(addprefix -I, $(shell find -L src/ -type d ))\
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
