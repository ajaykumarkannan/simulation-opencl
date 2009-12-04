DEPTH = ../../../../..

include $(DEPTH)/make/openclsdkdefs.mk 

####
#
#  Targets
#
####

CL_SAMPLE_EXE				= 1
INSTALL_TO_PUBLIC       	= 1
EXE_TARGET 					= BinarySearch
EXE_TARGET_INSTALL   		= BinarySearch
SRC_TARGET_INSTALL      	= BinarySearch.cpp BinarySearch.hpp BinarySearch_Kernels.cl BinarySearch.vcproj
SRC_DIR_INSTALL				= samples/opencl/cl/app/BinarySearch/

####
#
#  CPP files
#
####

CPPFILES 					= BinarySearch.cpp 
CLFILES						= BinarySearch_Kernels.cl
LLIBS  						+= SDKUtil
INCLUDEDIRS += $(SDK_HEADERS) 

include $(DEPTH)/make/openclsdkrules.mk 

