#Sets the root directory for this package
ROOTPLAT=../../..
#If User wants to build with Optimized_Assert builds, change LIBTYPE=Optimized_Assert
LIBTYPE=Optimized
#If User wants to build with Shared libraries, change LINKTYPE=Shared
LINKTYPE=

ARCHTYPE=$(shell uname -m)
ifeq ($(ARCHTYPE),i86pc)	
	#
	# This is x86 solaris SunStudio 12 64 bit
	#
	LIB_DIR=$(ROOTPLAT)/Libs/SOL10_X64_64_SS12/$(LIBTYPE)/$(LINKTYPE)
	OUTPUT_DIR=./SOL10_X64_64_SS12/$(LIBTYPE)/$(LINKTYPE)
	CFLAGS = -m64 -xO4 -DNDEBUG -xtarget=opteron -xarch=sse2 -nofstore -xchip=opteron -xarch=sse2 -xregs=frameptr -Di86pc -D_REENTRANT -Di86pc_SunOS_510_64 -KPIC -D_POSIX_PTHREAD_SEMANTICS 
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
			OUTPUT_DIR=./RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
			CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 
		else
			# assume AS6 (or greater)
			LIB_DIR=$(ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			OUTPUT_DIR=./RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
			CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=6 
		endif
	else 
		ifeq ($(findstring SUSE,$(DIST)),SUSE)	 # check for SUSE Linux
			ifeq ($(findstring 11, $(OS_VER)), 11)
				# use AS5 for SUSE 11 (or greater)
				LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				OUTPUT_DIR=./RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 
			else
				# if we dont know what it is, assume AS5 
				LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				OUTPUT_DIR=./RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
				CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 
			endif
		else
			ifeq ($(findstring CentOS,$(DIST)),CentOS)
				# assume Oracle 7 (or greater)
				LIB_DIR=$(ROOTPLAT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
				OUTPUT_DIR=./CENTOS7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
				CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=7
			else
				DIST=$(shell cat /etc/oracle-release)
				ifeq ($(findstring Oracle,$(DIST)),Oracle)  # check for Oracle Linux
					ifeq ($(findstring 5, $(OS_VER)), 5)
						#AS5 
						LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
						OUTPUT_DIR=./OL5_64_GCC412/$(LIBTYPE)/$(LINKTYPE)
						CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 
					else
						ifeq ($(findstring 6, $(OS_VER)), 6)
							# AS6
							LIB_DIR=$(ROOTPLAT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
							OUTPUT_DIR=./OL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
							CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=6 
						else
							# assume AS7 (or greater)
							LIB_DIR=$(ROOTPLAT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
							OUTPUT_DIR=./OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
							CFLAGS += -DLinux -DLINUX -Dx86_Linux_4X -Dx86_Linux_5X -Dx86_Linux_6X -DLinuxVersion=7
						endif
					endif
				endif
			endif
		endif
	endif
endif

CFLAGS += -D_SVID_SOURCE=1 -DCOMPILE_64BITS 
CLIBS = $(LIB_DIR)
LIB_PATH = -L$(LIB_DIR) 
HEADER_INC = -I$(ROOTPLAT)/Include/Codec

ifeq ($(LINKTYPE),Shared)
	LN_HOMEPATH=../../..
	RSSL_LIB = -lrsslVA -lrssl 
else
	LN_HOMEPATH=../..
	RSSL_LIB = $(LIB_DIR)/librssl.a 
endif
EXE_DIR=$(OUTPUT_DIR)

OS_NAME= $(shell uname -s)
ifeq ($(OS_NAME), SunOS)
	export CC=/tools/SunStudio12/SUNWspro/bin/cc
	SYSTEM_LIBS = -lnsl -lposix4 -lsocket -lrt -ldl -lm
	CFLAGS +=  -Dsun4_SunOS_5X -DSYSENT_H -DSOLARIS2 -xlibmil -xlibmopt -xbuiltin=%all -mt -R $(CLIBS) 
	LNCMD = ln -s
endif

ifeq ($(OS_NAME), Linux)
    export COMPILER=gnu
    export CC=/usr/bin/gcc -fPIC -ggdb3
	CFLAGS += -DNDEBUG -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_GNU_SOURCE -O3
	SYSTEM_LIBS = -lnsl -lpthread -lrt -ldl -lm
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


main: EncDecExample

EncDecExample_src = \
	rsslEncDecExample.c \
	mapEncDec.c \
	msgEncDec.c \
	elementListEncDec.c \
	seriesEncDec.c \
	filterListEncDec.c \
	vectorEncDec.c \
	fieldListEncDec.c 

EncDecExample_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(EncDecExample_src:%.c=%.o))
					

EncDecExample : $(EncDecExample_objs) 
	mkdir -p $(EXE_DIR)
	$(CC) -g -o $(EXE_DIR)/$@ $(EncDecExample_objs) $(CFLAGS) $(HEADER_INC) $(LIB_PATH) $(RSSL_LIB) $(SYSTEM_LIBS) 
	$(LNCMD) $(LN_HOMEPATH)/../../../etc/RDMFieldDictionary $(EXE_DIR)/RDMFieldDictionary
	$(LNCMD) $(LN_HOMEPATH)/../../../etc/enumtype.def $(EXE_DIR)/enumtype.def

clean:
	rm -rf $(OUTPUT_DIR)

