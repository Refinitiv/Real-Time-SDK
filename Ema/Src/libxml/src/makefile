#Sets the root directory for this package
ROOTPLAT=../
#Toggles 32 or 64 bit builds(32 = 32 bit libraries used, 64 = 64 bit libraries used)
COMPILE_BITS=64

# Because SS12 runs on the same platforms as SS10 and SS11, this toggles the compiler
# (0 = SS10 for 32-bit and SS11 for 64-bit, 1 = SS12 for both 32 and 64-bit)
USE_SS12=1

ARCHTYPE=$(shell uname -m)

cc=gcc
CFLAGS = -fPIC -DNDEBUG
ccARSHRD_FLAGS = -shared -Xlinker -rpath -Xlinker libs:. $(CCPPFLAGS) $(CCFLAGS)

ifeq ($(ARCHTYPE),i86pc)
	# This is x86 solaris
	ifeq ($(COMPILE_BITS),64)
		ifeq ($(USE_SS12),0)
			# This should be the Solaris 10 X86/X64 64 bit SunStudio 11 build
			LIB_DIR=$(ROOTPLAT)/Libs/SOL10_X64_64_SS11
			OUTPUT_DIR=./SOL10_X64_64_SS11/Static
			CFLAGS = -m64 -xtarget=opteron -xarch=sse2 -nofstore -xregs=frameptr -Di86pc -Di86pc_SunOS_510_64 -Xa -KPIC 
			CFLAGS += -nofstore -xregs=frameptr -xcache=64/64/2:1024/64/16
		else
			# This is the SunStudio 12 case
			# 64 bit SS12 case
			LIB_DIR=$(ROOTPLAT)/Libs/SOL10_X64_64_SS12
			OUTPUT_DIR=./SOL10_X64_64_SS12/Static
			CFLAGS = -m64 -xtarget=opteron -xarch=sse2 -nofstore -xregs=frameptr -Di86pc -Di86pc_SunOS_510_64 -Xa -KPIC 
			CFLAGS += -nofstore -xregs=frameptr
		endif
	else
		# This is the SunStudio 12 case
		# 32 bit SS12 case
		LIB_DIR=$(ROOTPLAT)/Libs/SOL10_X64_32_SS12
		OUTPUT_DIR=./SOL10_X64_32_SS12/Static
		CFLAGS = -m32 -xtarget=opteron -xarch=sse2 -nofstore -xregs=frameptr -Di86pc -Di86pc_SunOS_510 -Xa -KPIC 
		CFLAGS += -nofstore -xregs=frameptr
	endif
else
	# Assume this is some type of Linux 
	OS_VER=$(shell lsb_release -r | sed 's/\.[0-9]//')
	DIST=$(shell lsb_release -i)
	# Set flags based on 64 versus 32 bit machine
	ifeq ($(COMPILE_BITS),64)
		CFLAGS = -m64
	else
		CFLAGS = -m32
	endif

	# Check for RedHat Linux first
	ifeq ($(findstring RedHat,$(DIST)),RedHat)
		ifeq ($(findstring 5, $(OS_VER)), 5)
			# AS5
			LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_$(COMPILE_BITS)_GCC412
			OUTPUT_DIR=./RHEL5_$(COMPILE_BITS)_GCC412/Static
		else
			# Assume AS6 (or greater)
			LIB_DIR=$(ROOTPLAT)/Libs/RHEL6_$(COMPILE_BITS)_GCC444
			OUTPUT_DIR=./RHEL6_$(COMPILE_BITS)_GCC444/Static
			OUTPUT_DIR_SHARED=./RHEL6_$(COMPILE_BITS)_GCC444/Shared
		endif
	else
		# Check for SUSE Linux
		ifeq ($(findstring SUSE,$(DIST)),SUSE)
			# Use AS5 for SUSE 11 (or greater)
			LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_$(COMPILE_BITS)_GCC412
			OUTPUT_DIR=./RHEL5_$(COMPILE_BITS)_GCC412/Static
		else
			ifeq ($(findstring CentOS,$(DIST)),CentOS)
				LIB_DIR=$(ROOTPLAT)/Libs/OL7_$(COMPILE_BITS)_GCC482
				OUTPUT_DIR=./CENTOS7_$(COMPILE_BITS)_GCC482/Static
				OUTPUT_DIR_SHARED=./CENTOS7_$(COMPILE_BITS)_GCC482/Shared
			else
				DIST=$(shell cat /etc/oracle-release)
				ifeq ($(findstring Oracle,$(DIST)),Oracle)  # check for Oracle Linux
					ifeq ($(findstring 5, $(OS_VER)), 5)
						# AS5
						LIB_DIR=$(ROOTPLAT)/Libs/RHEL5_$(COMPILE_BITS)_GCC412
						OUTPUT_DIR=./OL5_$(COMPILE_BITS)_GCC412/Static
					else
						ifeq ($(findstring 6, $(OS_VER)), 6)
							# AS6
							LIB_DIR=$(ROOTPLAT)/Libs/RHEL6_$(COMPILE_BITS)_GCC444
							OUTPUT_DIR=./OL6_$(COMPILE_BITS)_GCC444/Static
							OUTPUT_DIR_SHARED=./OL6_$(COMPILE_BITS)_GCC444/Shared
						else
							# Assume AS7 (or greater)
							LIB_DIR=$(ROOTPLAT)/Libs/OL7_$(COMPILE_BITS)_GCC482
							OUTPUT_DIR=./OL7_$(COMPILE_BITS)_GCC482/Static
							OUTPUT_DIR_SHARED=./OL7_$(COMPILE_BITS)_GCC482/Shared
						endif
					endif
				endif
			endif
		endif
	endif
endif

ifeq ($(COMPILE_BITS),64)
	CFLAGS += -DCOMPILE_64BITS 
endif

CFLAGS += -D_SVID_SOURCE=1 

CFLAGS += -DHAVE_CONFIG_H -DLIBXML_THREAD_ENABLED
CLIBS = $(LIB_DIR)
HEADER_INC = -I./include

EXE_DIR=$(OUTPUT_DIR)
EXE_DIR_SHARED=$(OUTPUT_DIR_SHARED)
LN_HOMEPATH=../..

OS_NAME= $(shell uname -s)
ifeq ($(OS_NAME), SunOS)
	export CC=/tools/SunStudio12/SUNWspro/bin/cc
	CFLAGS +=  -Dsun4_SunOS_5X -DSYSENT_H -DSOLARIS2 -xlibmil -xlibmopt -xbuiltin=%all -mt -R $(CLIBS) 
	LNCMD = ln -s
	SOFLAGS= -G -Kpic
endif

ifeq ($(OS_NAME), Linux)
    export COMPILER=gnu
    export CC=/usr/bin/gcc -fPIC -ggdb3
	CFLAGS += -DLinux -Dx86_Linux_4X -Dx86_Linux_5X -DLinuxVersion=5 -D_iso_stdcpp_ -D_BSD_SOURCE=1 -D_POSIX_SOURCE=1 -D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE=500  -D_POSIX_PTHREAD_SEMANTICS -D_GNU_SOURCE
	LNCMD = ln -is
	SOFLAGS = -shared -fPIC
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


main: libxml2.a libxml2.so

libxml2_src = parserInternals.c parser.c tree.c hash.c list.c xmlIO.c \
	xmlmemory.c uri.c valid.c xlink.c HTMLparser.c HTMLtree.c \
	debugXML.c xpath.c xpointer.c xinclude.c nanohttp.c nanoftp.c \
	DOCBparser.c catalog.c globals.c threads.c c14n.c xmlstring.c \
	xmlregexp.c xmlschemas.c xmlschemastypes.c xmlunicode.c \
	xmlreader.c relaxng.c dict.c SAX2.c xmlwriter.c legacy.c \
	chvalid.c pattern.c xmlsave.c xmlmodule.c schematron.c \
	triostr.c trionan.c trio.c encoding.c entities.c error.c


libxml2_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(libxml2_src:%.c=%.o))

libxml2.a : $(libxml2_objs)
	mkdir -p $(EXE_DIR)
	ar -ruv $(EXE_DIR)/libxml2.a $(libxml2_objs)

libxml2.so : $(libxml2_objs)
	mkdir -p $(EXE_DIR_SHARED)
	$(CC) $(CFLAGS) $(SOFLAGS) -o $(EXE_DIR_SHARED)/libxml2.so $^
#$(ccARSHRD) $(ccARSHRD_FLAGS) $(SONAME_FLAGS)$(SONAME_NAME) $(ccARSHRD_OUT) $@ $(LIB_EXTRA_INC) $^

clean:
	rm -rf $(OUTPUT_DIR)
