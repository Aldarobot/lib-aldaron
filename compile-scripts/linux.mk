PLATFORM = Linux

$(OBJ_SDL): $(SRC_SDL)
	printf "[COMP] compiling SDL...\n"
	cd $(SRC_SDL)/ && sh configure --prefix=`pwd`/usr_local/ && make -j 4 && make install
	ld -r $(SRC_SDL)/build/.libs/*.o -o $(OBJ_SDL)
	printf "[COMP] done!\n"
