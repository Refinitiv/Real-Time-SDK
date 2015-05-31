#Sets the root directory for this package
ROOTPLAT=../../../..
ETA_ROOT=$(ROOTPLAT)/../$(shell ls ../../../../../ | grep eta)

#If User wants to build with Optimized_Assert builds, change LIBTYPE=Optimized_Assert
LIBTYPE=Optimized
#If User wants to build with Shared libraries, change LINKTYPE=Shared
LINKTYPE=

OS_VER = $(shell lsb_release -r | sed 's/\.[0-9]//')
DIST = $(shell lsb_release -i)
ifeq ($(findstring RedHat,$(DIST)), RedHat)
	ifeq ($(findstring 6, $(OS_VER)), 6)
		EMA_LIB_DIR = $(ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
		ETA_LIB_DIR = $(ETA_ROOT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
		ETA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
		OUTPUT_DIR = RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
	endif
else
	ifeq ($(findstring OracleServer, $(DIST)), OracleServer)
		ifeq ($(findstring 6, $(OS_VER)), 6)
			EMA_LIB_DIR = $(ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			OUTPUT_DIR = OL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			ETA_LIB_DIR = $(ETA_ROOT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			ETA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
		else
			EMA_LIB_DIR = $(ROOTPLAT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
			OUTPUT_DIR = OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
			ETA_LIB_DIR = $(ETA_ROOT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
			ETA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
		endif
	else
		ifeq ($(findstring CentOS, $(DIST)), CentOS)
			ifeq ($(findstring 7, $(OS_VER)), 7)
				EMA_LIB_DIR = $(ROOTPLAT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
				OUTPUT_DIR = CENTOSL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
				ETA_LIB_DIR = $(ETA_ROOT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
				ETA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
			endif
		endif
	endif
endif

ifndef EMA_LIB_DIR
$(error unsupported environment)
endif

ifeq ($(findstring Shared,$(LINKTYPE)), Shared)
	LIBS = -lema -lxml2 -lrssl -lrsslVA
	LN_HOMEPATH=../../../../../..
	LN_EMACFG_PATH=../../..
else
	LIBS = -lema
	LN_HOMEPATH=../../../../..
	LN_EMACFG_PATH=../..
endif
# 64bit only
CFLAGS += -m64 -D_REENTRANT -DLinux  -DCOMPILE_64BITS
LIB_PATH = -L$(EMA_LIB_DIR) -L$(ETA_LIB_DIR) -L$(ETA_VA_LIB_DIR)
HEADER_INC =	-I$(ROOTPLAT)/Src/Access/Include	\
				-I$(ROOTPLAT)/Src	\
				-I$(ROOTPLAT)/Src/Include	\
				-I$(ROOTPLAT)/Src/Rdm/Include
EMA_LIBS =  $(LIBS) 

SYSTEM_LIBS = -lnsl -lpthread -ldl -lrt 

EXE_DIR=$(OUTPUT_DIR)

CC=/usr/bin/g++
LNCMD = ln -is

$(OUTPUT_DIR)/obj/%.o : %.cpp 
	rm -f $@ 
	mkdir -p $(dir $@) 
	$(CC) -c $(CFLAGS) $(HEADER_INC) -o $@ $< 

Consumer_src = Consumer.cpp  
Consumer_head = Consumer.h  
  
Consumer_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(Consumer_src:%.cpp=%.o))  

