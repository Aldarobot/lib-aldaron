ifneq ("$(shell uname | grep Linux)", "")
 ifneq ("$(shell uname -m | grep arm)", "")
  # Build for Raspberry Pi
  PLATFORM = rpi
  PLATFORM_CFLAGS = -L/opt/vc/lib/ -lbcm_host -DJL_PLAT_RPI
 else
  # Build for Other Linux Target
  PLATFORM = linux
  PLATFORM_CFLAGS = 
 endif
else ifneq ("$(shell uname | grep Darwin)", "")
 # Build for Apple Computers ( Mac OS )
 PLATFORM = macos
 PLATFORM_CFLAGS = 
else
 $(error "Platform is not supported")
endif
