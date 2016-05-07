SRCLANG := c++

APPL_CXXOBJS += Walker.o Scanner.o Tower.o Control.o
APPL_COBJS += 

ifdef CONFIG_EV3RT_APPLICATION

# Include libraries
include $(EV3RT_SDK_LIB_DIR)/libcpp-ev3/Makefile

endif
