include ../../../../../makefile.common
include ../../makefile.examples.common

ifeq ($(findstring OracleServer, $(DIST)), OracleServer)
	ifeq ($(findstring 7, $(OS_VER)), 7)
		IDENT= OL7_64_GCC482
	endif
else
	ifeq ($(findstring CentOS, $(DIST)), CentOS)
		ifeq ($(findstring 7, $(OS_VER)), 7)
			IDENT= OL7_64_GCC482
		endif
	endif
endif

ifndef IDENT	
$(error unsupported environment)
endif

optimizedStatic:	OUTPUT_DIR := $(IDENT)/Optimized
optimizedAssertStatic:	OUTPUT_DIR := $(IDENT)/Optimized_Assert

optimizedStatic: outputDir
	$(MAKE) -f ../../makefile.second OUTPUT_DIR=$(OUTPUT_DIR) $@

optimizedAssertStatic: outputDir
	$(MAKE) -f ../../makefile.second OUTPUT_DIR=$(OUTPUT_DIR) $@

help:
	@echo "allowed build targets: optimizedStatic optimizedAssertStatic"
ifeq ($(findstring GSG,$(DISTRIBUTION_SRC)), GSG)
	@echo "use USE_CUSTOM_BUILD=true to use custom library build in examples"
endif
	@echo "allowed clean targets: clean optimizedStaticClean optimizedAssertStaticClean"
	@echo "target clean will clear all build directories for $(IDENT)"

outputDir:
	@mkdir -p $(OUTPUT_DIR)/obj

optimizedStaticClean:		OUTPUT_DIR := $(IDENT)/Optimized
optimizedStaticClean:
	$(foreach file, $(shell find $(OUTPUT_DIR) -type f -o -type l 2>/dev/null | grep -v Shared), /bin/rm $(file); )
	rmdir $(OUTPUT_DIR)/obj $(OUTPUT_DIR) $(IDENT) 2>/dev/null | true

optimizedAssertStaticClean:	OUTPUT_DIR := $(IDENT)/Optimized_Assert
optimizedAssertStaticClean:
	$(foreach file, $(shell find $(OUTPUT_DIR) -type f -o -type l 2>/dev/null | grep -v Shared), /bin/rm $(file); )
	rmdir $(OUTPUT_DIR)/obj $(OUTPUT_DIR) $(IDENT) 2>/dev/null | true

clean:
	/bin/rm -fr $(IDENT)
