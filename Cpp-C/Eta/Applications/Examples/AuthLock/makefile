#Sets the root directory for this package
ROOTPLAT=../../..

ARCHTYPE=$(shell uname -m)
ifeq ($(ARCHTYPE),i86pc)	
	#
	# This is x86 solaris SunStudio 12 64 bit
	#
	LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/SOL10_X64_64_SS12/Optimized
	OUTPUT_DIR=./SOL10_X64_64_SS12/Optimized
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
		   	LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/RHEL5_64_GCC412/Optimized
		   	OUTPUT_DIR=./RHEL5_64_GCC412/Optimized
		else
			# assume AS6 (or greater)
			LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/RHEL6_64_GCC444/Optimized
			OUTPUT_DIR=./RHEL6_64_GCC444/Optimized
		endif
	else 
		ifeq ($(findstring SUSE,$(DIST)),SUSE)	 # check for SUSE Linux
			ifeq ($(findstring 11, $(OS_VER)), 11)
				# use AS5 for SUSE 11 (or greater)
				LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/RHEL5_64_GCC412/Optimized
				OUTPUT_DIR=./RHEL5_64_GCC412/Optimized
			else
				# if we dont know what it is, assume AS5 
				LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/RHEL5_64_GCC412/Optimized
				OUTPUT_DIR=./RHEL5_64_GCC412/Optimized
			endif
		else
			ifeq ($(findstring CentOS,$(DIST)),CentOS)
				# assume Oracle 7
				LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/OL7_64_GCC482/Optimized
				OUTPUT_DIR=./CENTOS7_64_GCC482/Optimized
			else
				DIST=$(shell cat /etc/oracle-release)
				ifeq ($(findstring Oracle,$(DIST)),Oracle)  # check for Oracle Linux
					ifeq ($(findstring 5, $(OS_VER)), 5)
						#AS5 
						LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/RHEL5_64_GCC412/Optimized
						OUTPUT_DIR=./OL5_64_GCC412/Optimized
					else
						ifeq ($(findstring 6, $(OS_VER)), 6)
							# AS6
							LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/RHEL6_64_GCC444/Optimized
							OUTPUT_DIR=./OL6_64_GCC444/Optimized
						else
							# assume AS7 (or greater)
							LIB_DIR=$(ROOTPLAT)/Utils/Dacs/Libs/OL7_64_GCC482/Optimized
							OUTPUT_DIR=./OL7_64_GCC482/Optimized
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
HEADER_INC = -I$(ROOTPLAT)/Utils/Dacs/Include

DACS_LIB = $(LIB_DIR)/libdacs.a
EXE_DIR=$(OUTPUT_DIR)
LN_HOMEPATH=../..

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
	CFLAGS += -DLinux -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500  -D_POSIX_PTHREAD_SEMANTICS -D_GNU_SOURCE
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


main: authlock

rsslauthlock_src = \
	rsslAuthLock.c

rsslauthlock_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(rsslauthlock_src:%.c=%.o))
					
authlock : $(rsslauthlock_objs)
	mkdir -p $(EXE_DIR)
	$(CC) -g -o $(EXE_DIR)/$@ $(rsslauthlock_objs) $(CFLAGS) $(HEADER_INC) $(LIB_PATH) $(DACS_LIB) $(SYSTEM_LIBS) 

clean:
	rm -rf $(OUTPUT_DIR)

