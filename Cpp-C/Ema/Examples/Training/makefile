include ../../../../../makefile.common
include ../../../makefile.examples.common

# IDENT used for setting OUTPUT_DIR, which is passed to submakes
ifeq ($(findstring RedHat,$(DIST)), RedHat)
	ifeq ($(findstring 6, $(OS_VER)), 6)
		IDENT= RHEL6_64_GCC444
	endif
else
	ifeq ($(findstring OracleServer, $(DIST)), OracleServer)
		ifeq ($(findstring 6, $(OS_VER)), 6)
			IDENT= OL6_64_GCC444
		else
			IDENT= OL7_64_GCC482
		endif
	else
		ifeq ($(findstring CentOS, $(DIST)), CentOS)
			ifeq ($(findstring 7, $(OS_VER)), 7)
				IDENT= CENTOSL7_64_GCC482
			endif
		endif
	endif
endif

ifndef IDENT	
$(error unsupported environment)
endif

optimizedStatic:	OUTPUT_DIR := $(IDENT)/Optimized
optimizedStatic:	TARGET := optimizedStatic
optimizedShared:	OUTPUT_DIR := $(IDENT)/Optimized/Shared
optimizedShared:	TARGET := optimizedShared
optimizedAssertStatic:	OUTPUT_DIR := $(IDENT)/Optimized_Assert
optimizedAssertStatic:	TARGET := optimizedAssertStatic
optimizedAssertShared:	OUTPUT_DIR := $(IDENT)/Optimized_Assert/Shared
optimizedAssertShared:	TARGET := optimizedAssertShared

optimizedStatic: outputDir
	$(MAKE) -f ../../../makefile.second OUTPUT_DIR=$(OUTPUT_DIR) $(TARGET)

optimizedShared: outputDir
	$(MAKE) -f ../../../makefile.second OUTPUT_DIR=$(OUTPUT_DIR) $(TARGET)

optimizedAssertStatic: outputDir
	$(MAKE) -f ../../../makefile.second OUTPUT_DIR=$(OUTPUT_DIR) $(TARGET)

optimizedAssertShared: outputDir
	$(MAKE) -f ../../../makefile.second OUTPUT_DIR=$(OUTPUT_DIR) $(TARGET)

help:
	@echo "allowed build targets: optimizedStatic optimizedShared optimizedAssertStatic optimizedAssertShared"
ifeq ($(findstring GSG,$(DISTRIBUTION_SRC)), GSG)
	@echo "use USE_CUSTOM_BUILD=true to use custom library build in examples"
endif
	@echo "allowed clean targets: clean optimizedStaticClean optimizedSharedClean optimizedAssertStaticClean optimizedAssertSharedClean"
	@echo "target clean will clear all build directories for $(IDENT)"

outputDir:
	@mkdir -p $(OUTPUT_DIR)/obj

optimizedStaticClean:		OUTPUT_DIR := $(IDENT)/Optimized
optimizedStaticClean:
	$(foreach file, $(shell find $(OUTPUT_DIR) -type f -o -type l 2>/dev/null | grep -v Shared), /bin/rm $(file); )
	rmdir $(OUTPUT_DIR)/obj $(OUTPUT_DIR) $(IDENT) 2>/dev/null | true

optimizedSharedClean:		OUTPUT_DIR := $(IDENT)/Optimized/Shared
optimizedSharedClean:		
	/bin/rm -fr $(OUTPUT_DIR)
	rmdir $(shell dirname $(OUTPUT_DIR)) $(IDENT) 2>/dev/null | true

optimizedAssertStaticClean:	OUTPUT_DIR := $(IDENT)/Optimized_Assert
optimizedAssertStaticClean:
	$(foreach file, $(shell find $(OUTPUT_DIR) -type f -o -type l 2>/dev/null | grep -v Shared), /bin/rm $(file); )
	rmdir $(OUTPUT_DIR)/obj $(OUTPUT_DIR) $(IDENT) 2>/dev/null | true

optimizedAssertSharedClean:	OUTPUT_DIR := $(IDENT)/Optimized_Assert/Shared
optimizedAssertSharedClean:
	/bin/rm -fr $(OUTPUT_DIR)
	rmdir $(shell dirname $(OUTPUT_DIR)) $(IDENT) 2>/dev/null | true

clean:
	/bin/rm -fr $(IDENT)
