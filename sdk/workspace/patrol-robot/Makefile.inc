SRCLANG := c++

APPL_CXXOBJS += Walker.o Tower.o Control.o SmoothMotor.o Target.o
APPL_COBJS += 

ifdef CONFIG_EV3RT_APPLICATION

# Include libraries
include $(EV3RT_SDK_LIB_DIR)/libcpp-ev3/Makefile

endif
