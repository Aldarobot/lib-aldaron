PLATFORM = Raspberry Pi
LD = ld -r
WGET = wget
S = /

$(OBJ_SDL): $(SRC_SDL)
	# Compiling SDL for $(PLATFORM)....
	# Compiling SDL Dependencies....
	sudo apt-get update && \
	sudo apt-get install -y libudev-dev libevdev-dev\
		libasound2-dev \
		libdbus-1-dev libpng12-dev \
		libtiff5-dev libwebp-dev libvorbis-dev libflac-dev \
		--fix-missing
	# Compiling SDL....
	cd $(SRC_SDL)/ && sh configure --prefix=`pwd`/usr_local/ --host=armv7l-raspberry-linux-gnueabihf --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl --enable-libudev --enable-libevdev --enable-input-tslib && make -j 4 && make install
	$(LD) $(SRC_SDL)/build/.libs/*.o -o $(OBJ_SDL)
	# Done!\n"
