#If User wants to build with Optimized_Assert builds, change LIBTYPE = Optimized_Assert
export LIBTYPE = Optimized

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
	$(LNCMD) ../../Elektron-SDK-BinaryPack/Cpp-C/Eta/Libs ../
	$(LNCMD) ../../../../Elektron-SDK-BinaryPack/Cpp-C/Eta/Utils/Dacs/Libs ../Utils/Dacs/
	$(LNCMD) ../../../../Elektron-SDK-BinaryPack/Cpp-C/Eta/Utils/Ansi/Libs ../Utils/Ansi/
	make -f makefileVA

stubs:
	make -f makefileStubs
	
clean:
	make -f makefileVA clean
	
cleanstubs:
	make -f makefileStubs clean
	
all:
	make rsslVA
