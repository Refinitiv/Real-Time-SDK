#Include common makefile
include ../../common.mk

ConsumerManager_src = ConsumerManager.cpp

ConsumerManager_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(ConsumerManager_src:%.cpp=%.o))

main: Consumer

Consumer: $(Consumer_objs) $(ConsumerManager_objs)
	mkdir -p $(EXE_DIR)
	$(CC) -g -o $(EXE_DIR)/$@410 $(Consumer_objs) $(ConsumerManager_objs) $(CFLAGS) $(LIB_PATH) $(EMA_LIBS) $(SYSTEM_LIBS)
	$(LNCMD) $(LN_EMACFG_PATH)/EmaConfig.xml $(EXE_DIR)/EmaConfig.xml
	$(LNCMD) $(LN_HOMEPATH)/../Etc/RDMFieldDictionary $(EXE_DIR)/RDMFieldDictionary
	$(LNCMD) $(LN_HOMEPATH)/../Etc/enumtype.def $(EXE_DIR)/enumtype.def
clean:
	rm -rf $(OUTPUT_DIR)
