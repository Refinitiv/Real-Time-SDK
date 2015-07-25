#Sets the root UPA library directory for this package
RSSL_ROOTPLAT=../../..
#If User wants to build with Optimized_Assert builds, change to LIBTYPE=Optimized_Assert
LIBTYPE=Optimized
#If User wants to build with Shared libraries, change LINKTYPE=Shared
LINKTYPE=

ROOTPLAT=$(RSSL_ROOTPLAT)


ARCHTYPE=$(shell uname -m)
ifeq ($(ARCHTYPE),i86pc)		
	#
	# This is x86 solaris SunStudio 12 64 bit
	#
	LIB_DIR=$(ROOTPLAT)/Libs/SOL10_X64_64_SS12/$(LIBTYPE)/$(LINKTYPE)
	RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/SOL10_X64_64_SS12/$(LIBTYPE)/$(LINKTYPE)
	OUTPUT_DIR=./SOL10_X64_64_SS12/$(LIBTYPE)/$(LINKTYPE)
	CFLAGS = -m64 -xtarget=opteron -xarch=sse2 -nofstore -xregs=frameptr -Di86pc -Di86pc_SunOS_510_64 -KPIC 
else
	#
	# assume this is some type of Linux 
	#
	OS_VER=$(shell lsb_release -r | sed 's/\.[0-9]//')
	DIST=$(shell lsb_release -i)
	CFLAGS = -m64 

	# check for RedHat Linux first
	ifeq ($(findstring RedHat,$(DIST)),RedHat)
		ifeq ($(findstring 5, $(OS_VER)), 5)
       		#AS5 
		   	LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
		   	RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
		   	OUTPUT_DIR=./RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
		else
			# assume AS6 (or greater)
			LIB_DIR=$(ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			OUTPUT_DIR=./RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
		endif
	else 
		ifeq ($(findstring SUSE,$(DIST)),SUSE)	 # check for SUSE Linux
			ifeq ($(findstring 11, $(OS_VER)), 11)
				# use AS5 for SUSE 11 (or greater)
				LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				OUTPUT_DIR=./RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
			else
				# if we dont know what it is, assume AS5 
				LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				OUTPUT_DIR=./RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
			endif
		else
			ifeq ($(findstring CentOS,$(DIST)),CentOS)
				# assume Oracle 7 (or greater)
				RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
				LIB_DIR=$(RSSL_LIB_DIR)
				OUTPUT_DIR=./CENTOS7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
			else
				DIST=$(shell cat /etc/oracle-release)
				ifeq ($(findstring Oracle,$(DIST)),Oracle)  # check for Oracle Linux
					ifeq ($(findstring 5, $(OS_VER)), 5)
						#AS5 			
						RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
						LIB_DIR=$(RSSL_LIB_DIR)
						OUTPUT_DIR=./OL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
					else
						ifeq ($(findstring 6, $(OS_VER)), 6)
							# AS6
							RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
							LIB_DIR=$(RSSL_LIB_DIR)
							OUTPUT_DIR=./OL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
						else
							# assume AS7 (or greater)
							RSSL_LIB_DIR=$(RSSL_ROOTPLAT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
							LIB_DIR=$(RSSL_LIB_DIR)
							OUTPUT_DIR=./OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
						endif
					endif
				endif
			endif
		endif
	endif
endif

CFLAGS += -D_SVID_SOURCE=1 -DCOMPILE_64BITS 
VA_CLIBS = $(LIB_DIR) 
RSSL_CLIBS = $(RSSL_LIB_DIR)
LIB_PATH = -L$(LIB_DIR)
RSSL_LIB_PATH = -L$(RSSL_LIB_DIR)
HEADER_INC = -I../VACommon -I$(RSSL_ROOTPLAT)/Include/Cache -I$(RSSL_ROOTPLAT)/Include/Codec -I$(RSSL_ROOTPLAT)/Include/Transport -I$(RSSL_ROOTPLAT)/Include/RDM -I$(RSSL_ROOTPLAT)/Include/Reactor -I$(RSSL_ROOTPLAT)/Include/Util

ifeq ($(LINKTYPE),Shared)
    RSSL_VALUEADD_LIB = -lrsslVA -lrsslVACache
	RSSL_LIB = -lrssl
	LN_HOMEPATH=../../..
else
	RSSL_VALUEADD_LIB = $(LIB_DIR)/librsslReactor.a $(LIB_DIR)/librsslRDM.a $(LIB_DIR)/librsslVAUtil.a $(LIB_DIR)/librsslVACache.a
	RSSL_LIB = $(RSSL_LIB_DIR)/librssl.a
	LN_HOMEPATH=../..
endif

EXE_DIR=$(OUTPUT_DIR)

OS_NAME= $(shell uname -s)
ifeq ($(OS_NAME), SunOS)
	export CC=/tools/SunStudio12/SUNWspro/bin/cc
	SYSTEM_LIBS = -lnsl -lposix4 -lsocket -lrt -ldl -lm -lCrun
	CFLAGS +=  -Dsun4_SunOS_5X -DSYSENT_H -DSOLARIS2 -xlibmil -xlibmopt -xbuiltin=%all -mt -R $(VA_CLIBS) -R $(RSSL_CLIBS)
	LNCMD = ln -s
endif

ifeq ($(OS_NAME), Linux)
    export COMPILER=gnu
    export CC=/usr/bin/gcc -fPIC -ggdb3
	CFLAGS += -DLinux -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500  -D_POSIX_PTHREAD_SEMANTICS -D_GNU_SOURCE
	SYSTEM_LIBS = -lnsl -lpthread -lrt -ldl -lm -lstdc++
	LNCMD = ln -is
endif

$(OUTPUT_DIR)/obj/Dummy/%.o : %.c
	rm -f $@
	mkdir -p $(dir $@)
	$(CC)  -c $(CFLAGS) $(HEADER_INC) -o $@ $<

$(OUTPUT_DIR)/obj/%.o : %.c
	rm -f $@
	mkdir -p $(dir $@)
	$(CC)  -c $(CFLAGS) $(HEADER_INC) -o $@ $<


$(OUTPUT_DIR)/%.o : %.c
	rm -f $@
	mkdir -p $(dir $@)
	$(CC)  -c $(CFLAGS) $(HEADER_INC) -o $@ $<


main: vaprovider

rsslvaprovider_src = \
	rsslDictionaryProvider.c \
	rsslDirectoryHandler.c \
	rsslLoginHandler.c \
	rsslItemHandler.c \
	rsslProvider.c

rsslvaprovider_common = \
	../VACommon/rsslVAMarketPriceItems.c \
	../VACommon/rsslVAMarketByOrderItems.c \
	../VACommon/rsslVAMarketByPriceItems.c \
	../VACommon/rsslVASymbolListItems.c \
	../VACommon/rsslVAItemEncode.c \
	../VACommon/rsslVASendMessage.c \
	../VACommon/rsslVACacheHandler.c \
	../VACommon/simpleTunnelMsgHandler.c \
	../VACommon/tunnelStreamHandler.c

rsslvaprovider_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(rsslvaprovider_src:%.c=%.o))
					
rsslvaprovider_objs2 = $(addprefix $(OUTPUT_DIR)/obj/Dummy/,$(rsslvaprovider_common:%.c=%.o))

vaprovider : $(rsslvaprovider_objs) $(rsslvaprovider_objs2) 
	mkdir -p $(EXE_DIR)
	$(CC) -g -o $(EXE_DIR)/$@ $(rsslvaprovider_objs) $(rsslvaprovider_objs2) $(CFLAGS) $(HEADER_INC) $(LIB_PATH) $(RSSL_LIB_PATH) $(RSSL_VALUEADD_LIB) $(RSSL_LIB) $(SYSTEM_LIBS) 
	$(LNCMD) $(LN_HOMEPATH)/../../../etc/RDMFieldDictionary $(EXE_DIR)/RDMFieldDictionary
	$(LNCMD) $(LN_HOMEPATH)/../../../etc/enumtype.def $(EXE_DIR)/enumtype.def

clean:
	rm -rf $(OUTPUT_DIR)

