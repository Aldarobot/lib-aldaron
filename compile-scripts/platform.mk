ifneq ("$(shell uname | grep Linux)", "")
 ifneq ("$(shell uname -m | grep arm)", "")
  # Build for Raspberry Pi
  PLATFORM = rpi
  PLATFORM_CFLAGS = -L/opt/vc/lib/ -lbcm_host -DJL_PLAT_RPI
  PLATFORM_INCLUDES = -I/opt/vc/include
  S = "/"
  LN = ln -s $(LA_HOME)/src/lib-aldaron src/
  GL_VERSION=-lGLESv2
 else
  # Build for Other Linux Target
  PLATFORM = linux
  PLATFORM_CFLAGS =
  PLATFORM_INCLUDES =
  S = "/"
  LN = ln -s $(LA_HOME)/src/lib-aldaron src/
  GL_VERSION=-lGLESv2
 endif
else ifneq ("$(shell uname | grep Darwin)", "")
 # Build for Apple Computers ( Mac OS )
 PLATFORM = macos
 PLATFORM_CFLAGS =
 PLATFORM_INCLUDES =
 S = "/"
 LN = ln -s $(LA_HOME)/src/lib-aldaron src/
 GL_VERSION=-lGLESv2
else
 PLATFORM = windows
 PLATFORM_CFLAGS =
 PLATFORM_INCLUDES =
 S = "\\"
 LN = ln -s $(LA_HOME)/src/lib-aldaron src/
 GL_VERSION=-lopengl32
endif
