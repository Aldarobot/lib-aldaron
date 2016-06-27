ifneq ("$(shell uname | grep Linux)", "")
 ifneq ("$(shell uname -m | grep arm)", "")
  # Build for Raspberry Pi
  PLATFORM = rpi
  PLATFORM_CFLAGS = -L/opt/vc/lib/ -lbcm_host -DJL_PLAT_RPI
  PLATFORM_INCLUDES = -I/opt/vc/include
 else
  # Build for Other Linux Target
  PLATFORM = linux
  PLATFORM_CFLAGS = 
  PLATFORM_INCLUDES = 
 endif
else ifneq ("$(shell uname | grep Darwin)", "")
 # Build for Apple Computers ( Mac OS )
 PLATFORM = macos
 PLATFORM_CFLAGS = 
 PLATFORM_INCLUDES = 
else
 $(error "Platform is not supported")
endif
