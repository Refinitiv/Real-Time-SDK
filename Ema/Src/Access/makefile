include ../../makefile.common

# IDENT used for setting OUTPUT_DIR, which is passed to submakes
ifeq ($(findstring RedHat,$(DIST)), RedHat)
	ifeq ($(findstring 6, $(OS_VER)), 6)
		IDENT= RHEL6_64_GCC444
	endif
else
	ifeq ($(findstring OracleServer, $(DIST)), OracleServer)
		ifeq ($(findstring 7, $(OS_VER)), 7)
			IDENT= OL7_64_GCC482
		endif
	endif
endif

ifndef IDENT	
$(error unsupported environment)
endif

optimizedStatic:	OUTPUT_DIR := Libs/$(IDENT)/Optimized
optimizedShared:	OUTPUT_DIR := Libs/$(IDENT)/Optimized/Shared
optimizedAssertStatic:	OUTPUT_DIR := Libs/$(IDENT)/Optimized_Assert
optimizedAssertShared:	OUTPUT_DIR := Libs/$(IDENT)/Optimized_Assert/Shared

optimizedStatic: outputDir xml
	$(MAKE) -f makefile.lib OUTPUT_DIR=$(OUTPUT_DIR) $@
optimizedShared: outputDir xml
	$(MAKE) -f makefile.lib OUTPUT_DIR=$(OUTPUT_DIR) $@
optimizedAssertStatic:	outputDir xml
	$(MAKE) -f makefile.lib OUTPUT_DIR=$(OUTPUT_DIR) $@
optimizedAssertShared:	outputDir xml
	$(MAKE) -f makefile.lib OUTPUT_DIR=$(OUTPUT_DIR) $@
xml:
	$(MAKE) -C ../libxml/src

all:
	$(foreach T, optimizedStatic optimizedShared optimizedAssertStatic optimizedAssertShared, $(MAKE) $T;)

help:
	@echo "allowed build targets: optimizedStatic (default), optimizedShared, optimizedAssertStatic, optimizedAssertShared all"
	@echo "allowed clean targets: clean optimizedStaticClean optimizedSharedClean optimizedAssertStaticClean optimizedAssertSharedClean"
	@echo "target clean will clear all build directories for $(IDENT)"

outputDir:
	mkdir -p $(OUTPUT_DIR)/obj/Access/Impl $(OUTPUT_DIR)/obj/Rdm/Impl

optimizedStaticClean:		OUTPUT_DIR := Libs/$(IDENT)/Optimized
optimizedStaticClean:
	/bin/rm -fr $(OUTPUT_DIR)/obj $(OUTPUT_DIR)/libema.a
	rmdir $(OUTPUT_DIR) Libs/$(IDENT) Libs 2>/dev/null | true

optimizedSharedClean:		OUTPUT_DIR := Libs/$(IDENT)/Optimized/Shared
optimizedSharedClean:		
	/bin/rm -fr $(OUTPUT_DIR)/obj $(OUTPUT_DIR)/libema.so
	rmdir $(OUTPUT_DIR) $(shell dirname $(OUTPUT_DIR)) Libs/$(IDENT) Libs 2>/dev/null | true

optimizedAssertStaticClean:	OUTPUT_DIR := Libs/$(IDENT)/Optimized_Assert
optimizedAssertStaticClean:
	/bin/rm -fr $(OUTPUT_DIR)/obj $(OUTPUT_DIR)/libema.a
	rmdir $(OUTPUT_DIR) Libs/$(IDENT) Libs 2>/dev/null | true

optimizedAssertSharedClean:	OUTPUT_DIR := Libs/$(IDENT)/Optimized_Assert/Shared
optimizedAssertSharedClean:
	/bin/rm -fr $(OUTPUT_DIR)/obj $(OUTPUT_DIR)/libema.so
	rmdir $(OUTPUT_DIR) $(shell dirname $(OUTPUT_DIR)) Libs/$(IDENT) Libs 2>/dev/null | true

clean:
	/bin/rm -fr Libs/$(IDENT)

