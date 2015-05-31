#Include common makefile
include ../../common.mk

main: Consumer

Consumer: $(Consumer_objs)
	mkdir -p $(EXE_DIR)
	$(CC) -g -o $(EXE_DIR)/$@400 $(Consumer_objs) $(CFLAGS) $(LIB_PATH) $(EMA_LIBS) $(SYSTEM_LIBS) 
	$(LNCMD) $(LN_EMACFG_PATH)/EmaConfig.xml $(EXE_DIR)/EmaConfig.xml
	$(LNCMD) $(LN_HOMEPATH)/../Etc/RDMFieldDictionary $(EXE_DIR)/RDMFieldDictionary
	$(LNCMD) $(LN_HOMEPATH)/../Etc/enumtype.def $(EXE_DIR)/enumtype.def
clean:
	rm -rf $(OUTPUT_DIR)
