include ../Common/makefile.common

main: .PHONY EmaCppConsPerf

.PHONY: $(XML_LIB_SRC)
$(XML_LIB_SRC):
	@$(MAKE) -C $@ COMPILE_FLAGS=64; 

EmaCppConsPerf_src = \
	ConsPerfConfig.cpp \
	EmaCppConsPerf.cpp \
	ConsumerThread.cpp \
	../Common/Statistics.cpp \
	../Common/AppUtil.cpp \
	../Common/CtrlBreakHandler.cpp \
	../Common/GetTime.cpp \
	../Common/Mutex.cpp \
	../Common/PerfConfig.cpp \
	../Common/ThreadAffinity.cpp \
	../Common/XmlItemParser.cpp

EmaCppConsPerf_objs = $(addprefix $(OUTPUT_DIR)/obj/,$(EmaCppConsPerf_src:%.cpp=%.o))

EmaCppConsPerf : $(EmaCppConsPerf_objs)
	mkdir -p $(EXE_DIR)
	$(CC) -o $(EXE_DIR)/$@ $(EmaCppConsPerf_objs) $(CFLAGS) $(HEADER_INC) $(LIB_PATH) $(EMA_LIB) $(SYSTEM_LIBS) 
	$(LNCMD) $(LN_HOMEPATH)/../Common/MsgData.xml $(EXE_DIR)
	$(LNCMD) $(LN_HOMEPATH)/../Common/350k.xml $(EXE_DIR)
	$(LNCMD) $(LN_HOMEPATH)/../EmaConfig.xml $(EXE_DIR)
	$(LNCMD) $(LN_HOMEPATH)/../../../Etc/RDMFieldDictionary $(EXE_DIR)
	$(LNCMD) $(LN_HOMEPATH)/../../../Etc/enumtype.def $(EXE_DIR)

clean:
	rm -rf $(OUTPUT_DIR)
