include ../Common/makefile.common

appName= EmaCppConsPerf

optimizedStatic:	OUTPUT_DIR = $(IDENT)/Optimized
optimizedStatic: outputDir
	cd ../Common;make $@
	make OUTPUT_DIR=$(OUTPUT_DIR) LIBS="$(LIBS)" $(appName)

optimizedShared:	OUTPUT_DIR = $(IDENT)/Optimized/Shared
optimizedShared:	LIBS+= -lrsslVA -lrssl
optimizedShared: outputDir
	cd ../Common;make $@
	make OUTPUT_DIR=$(OUTPUT_DIR) LIBS="$(LIBS)" ETA_SHARED_LIB_PATH="$(ETA_SHARED_LIB_PATH)" $(appName)

optimizedAssertStatic:	OUTPUT_DIR = $(IDENT)/Optimized_Assert
optimizedAssertStatic: outputDir
	cd ../Common;make $@
	make OUTPUT_DIR=$(OUTPUT_DIR) LIBS="$(LIBS)" $(appName)

optimizedAssertShared:	OUTPUT_DIR = $(IDENT)/Optimized_Assert/Shared
optimizedAssertShared:	LIBS+= -lrsslVA -lrssl
optimizedAssertShared: outputDir
	cd ../Common;make $@
	make OUTPUT_DIR=$(OUTPUT_DIR) LIBS="$(LIBS)" ETA_SHARED_LIB_PATH="$(ETA_SHARED_LIB_PATH)" $(appName)

.PHONY:  all
all:
	$(foreach T, optimizedStatic optimizedShared optimizedAssertStatic optimizedAssertShared, $(MAKE) $T;)

outputDir:
	@mkdir -p $(OUTPUT_DIR)/obj

clean:
	/bin/rm -fr $(IDENT)

SRC=	$(shell ls *.cpp)
OBJS=   $(SRC:%.cpp=$(OUTPUT_DIR)/obj/%.o)
LIBS = -lema -lPerfTools

ifeq ($(USE_CUSTOM_BUILD),true)
ifeq ($(IDENT),OL6_64_GCC444)
$(error custom build not supported for OL6)
else ifeq ($(IDENT),CENTOS7_64_GCC482)
$(error custom build not supported for CENTOS7)
else				#USE_CUSTOM_BUILD true; supported environment
	EMA_LIB_PATH=-L$(ROOTNODE)/Ema/Src/Access/Libs/$(OUTPUT_DIR)
endif
else				#USE_CUSTOM_BUILD false
	EMA_LIB_PATH=-L$(ROOTNODE)/Ema/Libs/$(OUTPUT_DIR)
endif

ETA_SHARED_LIB_PATH=-L$(ROOTNODE)/Eta/Libs/$(OUTPUT_DIR)
# libema not built for OL6, so use RHEL6; also not built for CENTOS, so use OL7
ifeq ($(IDENT), OL6_64_GCC444)
	ALTERNATE_LIBEMA_LOC= -L$(ROOTNODE)/Ema/Libs/$(subst OL6,RHEL6,$(OUTPUT_DIR))
	ETA_SHARED_LIB_PATH=$(subst OL6,RHEL6,-L$(ROOTNODE)/Eta/Libs/$(OUTPUT_DIR))
else
	ifeq ($(IDENT), CENTOS7_64_GCC482)
		ALTERNATE_LIBEMA_LOC= -L$(ROOTNODE)/Ema/Libs/$(subst CENTOS7,OL7,$(OUTPUT_DIR))
		ETA_SHARED_LIB_PATH=$(subst CENTOS7,OL7,-L$(ROOTNODE)/Eta/Libs/$(OUTPUT_DIR))
	endif
endif

#LIBPATH=   -L$(ROOTNODE)/Ema/Libs/$(OUTPUT_DIR) $(ALTERNATE_LIBEMA_LOC) -L../Common/$(OUTPUT_DIR)/obj  $(ETA_SHARED_LIB_PATH)
LIBPATH=   $(EMA_LIB_PATH) $(ALTERNATE_LIBEMA_LOC) -L../Common/$(OUTPUT_DIR)/obj  $(ETA_SHARED_LIB_PATH)
SYSTEM_LIBS = -lnsl -lpthread -lrt -ldl -lm
LNCMD = ln -is

$(appName) : $(OBJS)
	$(CC) -o $(OUTPUT_DIR)/$@ $(OBJS) $(CFLAGS) $(LIBPATH) $(LIBS) $(SYSTEM_LIBS) 
	$(LNCMD) $(ROOTNODE)/Ema/Examples/PerfTools/Common/MsgData.xml $(OUTPUT_DIR)
	$(LNCMD) $(ROOTNODE)/Ema/Examples/PerfTools/Common/350k.xml $(OUTPUT_DIR)
	$(LNCMD) $(ROOTNODE)/Ema/Examples/PerfTools/EmaConfig.xml $(OUTPUT_DIR)
	$(LNCMD) $(ROOTNODE)/Ema/Etc/RDMFieldDictionary $(OUTPUT_DIR)
	$(LNCMD) $(ROOTNODE)/Ema/Etc/enumtype.def $(OUTPUT_DIR)

$(OUTPUT_DIR)/obj/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
