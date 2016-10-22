PLATFORM = Linux
LD = ld -r
S = "/"
LN = ln -s

$(OBJ_SDL): $(SRC_SDL)
	printf "[COMP] compiling SDL...\n"
	cd $(SRC_SDL)/ && sh configure --disable-video-wayland --prefix=`pwd`/usr_local/ && make -j 4 && make install
	$(LD) $(SRC_SDL)/build/.libs/*.o -o $(OBJ_SDL)
	printf "[COMP] done!\n"
