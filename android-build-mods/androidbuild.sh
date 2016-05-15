#!/bin/bash
#
# This is a modified verion of SDL's androidbuild.sh
#

SOURCES=()
CURDIR=`pwd -P`

# Fetch sources
if [[ $# -ge 2 ]]; then
	for src in ${@:2}
	do
		SOURCES+=($src)
	done
else
	if [ -n "$1" ]; then
		while read src
		do
			SOURCES+=($src)
		done
	fi
fi

if [ -z "$1" ] || [ -z "$SOURCES" ]; then
	echo "Usage: androidbuild.sh com.yourcompany.yourapp < sources.list"
	echo "Usage: androidbuild.sh com.yourcompany.yourapp source1.c source2.c ...sourceN.c"
	echo "You can pass additional arguments to ndk-build with the NDKARGS variable: NDKARGS=\"-s\" androidbuild.sh ..."
	exit 1
fi

SDLPATH="$( cd "$(dirname "$0")/.." ; pwd -P )"
SDLIMAGEPATH=$SDLPATH/../SDL2_image-2.0.1/
SDLMIXERPATH=$SDLPATH/../SDL2_mixer-2.0.1/
SDLNETPATH=$SDLPATH/../SDL2_net-2.0.1/
ZIPPATH=$SDLPATH/../libzip-1.1.2/
MIKMODPATH=$SDLMIXERPATH/external/libmikmod-3.1.12/
SMPEG2PATH=$SDLMIXERPATH/external/smpeg2-2.0.0/
CLUMPPATH=$SDLPATH/../../src/lib/clump/

NDKBUILD=`which ndk-build`
if [ -z "$NDKBUILD" ];then
	echo "Could not find ndk-build, install Android's NDK and add it to the path"
	exit 1
fi

ANDROID=`which android`
if [ -z "$ANDROID" ];then
	echo "Could not find android, install Android's SDK and add it to the path"
	exit 1
fi

ANT=`which ant`

if [ -z "$ANT" ];then
	echo "Could not find ant, install Android's SDK and add it to the path"
	exit 1
fi

ZIPALIGN=`which zipalign`
if [ -z "$ZIPALIGN" ];then
	echo "Could not find zipalign, install Android's SDK and add it to the path"
	exit 1
fi

JARSIGNER=`which jarsigner`
if [ -z "$JARSIGNER" ];then
	echo "Could not find jarsigner, install java's jarsigner"
	exit 1
fi

ADB=`which adb`
if [ -z "$ADB" ];then
	echo "Could not find adb, install Android's SDK and add it to the path"
	exit 1
fi

KEYTOOL=`which keytool`
if [ -z "$KEYTOOL" ];then
	echo "Could not find keytool, install java's keytool"
	exit 1
fi

NCPUS="1"
case "$OSTYPE" in
	darwin*)
		NCPU=`sysctl -n hw.ncpu`
		;; 
	linux*)
		if [ -n `which nproc` ]; then
			NCPUS=`nproc`
		fi  
		;;
  *);;
esac

APP="$1"
APPARR=(${APP//./ })
BUILDPATH="$SDLPATH/build/jl_lib-android-project"

# Start Building

if [ "$1" = "_" ]; then
	rm -rf $BUILDPATH
	mkdir -p $BUILDPATH

	cp -r $SDLPATH/android-project/* $BUILDPATH

	# Link SDL sources
	mkdir -p $BUILDPATH/jni/SDL
	ln -s $SDLPATH/src $BUILDPATH/jni/SDL
	ln -s $SDLPATH/include $BUILDPATH/jni/SDL

	cp -r $SDLPATH/Android.mk $BUILDPATH/jni/SDL

	# Link SDL_image sources
	ln -sTf $SDLIMAGEPATH $BUILDPATH/jni/SDL_image

	# Link SDL_mixer sources
	ln -sTf $SDLMIXERPATH $BUILDPATH/jni/SDL_mixer
	# Dependencies
	ln -sTf $MIKMODPATH $BUILDPATH/jni/mikmod
	ln -sTf $SMPEG2PATH $BUILDPATH/jni/smpeg2

	# Link SDL_net sources
	ln -sTf $SDLNETPATH $BUILDPATH/jni/SDL_net

	# Link Lib Zip sources
	cp -u $ZIPPATH/config.h $ZIPPATH/lib/config.h
	ln -sTf $ZIPPATH/lib/ $BUILDPATH/jni/libzip

	# Link Clump sources
	ln -sTf $CLUMPPATH $BUILDPATH/jni/clump
else
	# Copy Top level files only
	cp $SDLPATH/android-project/* $BUILDPATH >/dev/null 2>/dev/null
fi

# Link user sources
rm -rf $BUILDPATH/jni/src/
mkdir -p $BUILDPATH/jni/src/

cp $SDLPATH/android-project/jni/src/Android.mk $BUILDPATH/jni/src/

for src in "${SOURCES[@]}"
do
	ln -fs $src $BUILDPATH/jni/src
done

# Create an inherited Activity
cd $BUILDPATH/src
for folder in "${APPARR[@]}"
do
	mkdir -p $folder
	cd $folder
done

ACTIVITY="${folder}Activity"
sed -i "s|org\.libsdl\.app|$APP|g" $BUILDPATH/AndroidManifest.xml
sed -i "s|SDLActivity|$ACTIVITY|g" $BUILDPATH/AndroidManifest.xml
sed -i "s|SDLActivity|$APP|g" $BUILDPATH/build.xml

# Fill in a default Activity
echo "package $APP;" >  "$ACTIVITY.java"
echo "import org.libsdl.app.SDLActivity;" >> "$ACTIVITY.java"
echo "public class $ACTIVITY extends SDLActivity {}" >> "$ACTIVITY.java"

# Update project and build
cd $BUILDPATH

# $ANDROID list targets
$ANDROID update project --path $BUILDPATH --target 1
$NDKBUILD -j $NCPUS $NDKARGS

# Check for release key, and make if doesn't exist
if [ ! -e $CURDIR/build/android-release-key.keystore ];then
	printf "[JL/ANDR] Jarsigner key not found.  For android you must create\n"
	printf "[JL/ANDR] a key.  Create your key for jarsigner:\n"
	keytool -sigalg SHA1withRSA -keyalg RSA -keysize 1024 -genkey -keystore $CURDIR/build/android-release-key.keystore -alias daliasle -validity 3650
fi

# Build and install
APK="$BUILDPATH/bin/$APP-release.apk"

$ANT release

$JARSIGNER -verbose -tsa http://timestamp.digicert.com -sigalg SHA1withRSA -digestalg SHA1 -keystore $CURDIR/build/android-release-key.keystore bin/$APP-release-unsigned.apk daliasle

rm -f $APK
$ZIPALIGN -v 4 bin/$APP-release-unsigned.apk $APK

if [ -f "$APK" ]; then
	$ADB install -r $APK
	$ADB shell am start -a android.intent.action.MAIN -n $APP/.$ACTIVITY
	$ADB logcat | grep I\/SDL\/APP
	exit 0
fi

echo "There was an error building the APK"
exit 1
