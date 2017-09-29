#If User wants to build with Optimized_Assert builds, change LIBTYPE = Optimized_Assert
export LIBTYPE = $(arg1)

OS_NAME= $(shell uname -s)
ifeq ($(OS_NAME), SunOS)
	LNCMD = ln -sfn
endif

ifeq ($(OS_NAME), Linux)
	LNCMD = ln -fs
endif


main: rsslVA

#default build expects to use binary libraries for now
#this will eventually build the full code base when it is open sourced
#it depends on git submodule to pull Elektron-SDK-BinaryPack repository as 
#well and then links to the libraries folder.
rsslVA:
		if [ ! -d ../Libs/RHEL* ]; then \
		if [ ! -d ../Libs/OL* ]; then \
			echo "Now softlink Elektron-SDK-BinaryPack dir to the libraries folder..."; \
			$(LNCMD) ../../Elektron-SDK-BinaryPack/Cpp-C/Eta/Libs ../; \
			$(LNCMD) ../../../../Elektron-SDK-BinaryPack/Cpp-C/Eta/Utils/Dacs/Libs ../Utils/Dacs/; \
			$(LNCMD) ../../../../Elektron-SDK-BinaryPack/Cpp-C/Eta/Utils/Ansi/Libs ../Utils/Ansi/; \
		fi; \
		fi
		gmake -f makefileVA
	
stubs:
	gmake -f makefileStubs
	
clean:
	gmake -f makefileVA clean
	
cleanstubs:
	gmake -f makefileStubs clean
	
all:
	gmake rsslVA
