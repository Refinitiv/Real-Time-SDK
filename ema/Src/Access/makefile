# Sets the root directory for the eta and ema packages
ETA_ROOT = ../../../eta
EMA_ROOT = ../../../ema

# LIBTYPE choices:
# LIBTYPE=Optimized_Assert	-> links with ETA optimized assert libs
# LIBTYPE=Optimized			-> links with ETA optimized libs
LIBTYPE=Optimized

#LINKTYPE choices
# LINKTYPE=Shared		-> builds shared EMA lib
# LINKTYPE=				-> builds static EMA lib
LINKTYPE=

# Set up some useful variables
STATIC_SUF = .a
SHARED_SUF = .so
OS_NAME = $(shell uname -s)
ARCHTYPE = $(shell uname -m)

OS_VER = $(shell lsb_release -r | sed 's/\.[0-9]//')
DIST = $(shell lsb_release -i)
ifeq ($(findstring RedHat,$(DIST)), RedHat)
    ifeq ($(findstring 6, $(OS_VER)), 6)
        EMA_LIB_DIR = $(EMA_ROOT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
        UPA_LIB_DIR = $(ETA_ROOT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
        UPA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
        OUTPUT_DIR = Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
    endif
else
    ifeq ($(findstring OracleServer, $(DIST)), OracleServer)
        ifeq ($(findstring 6, $(OS_VER)), 6)
            EMA_LIB_DIR = $(EMA_ROOT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
            UPA_LIB_DIR = $(ETA_ROOT)/Libs/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
            UPA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/RHEL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
            OUTPUT_DIR = Libs/OL6_64_GCC444/$(LIBTYPE)/$(LINKTYPE)
        else
            EMA_LIB_DIR = $(EMA_ROOT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
            UPA_LIB_DIR = $(ETA_ROOT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
            UPA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
            OUTPUT_DIR = Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
        endif
    else
        ifeq ($(findstring CentOS, $(DIST)), CentOS)
            ifeq ($(findstring 7, $(OS_VER)), 7)
                EMA_LIB_DIR = $(EMA_ROOT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
                UPA_LIB_DIR = $(ETA_ROOT)/Libs/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
                UPA_VA_LIB_DIR = $(ETA_ROOT)/Libs/ValueAdd/OL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
                OUTPUT_DIR = Libs/CENTOSL7_64_GCC482/$(LIBTYPE)/$(LINKTYPE)
            endif
        endif
    endif
endif

ifndef UPA_LIB_DIR
$(error unsupported environment)
endif

#add libs and includes
LIB_PATH = -L$(UPA_LIB_DIR) -L$(UPA_VA_LIB_DIR) -L$(EMA_LIB_DIR)
HEADER_INC = \
	-I.. \
	-IInclude \
	-IImpl \
	-I../Rdm/Impl \
	-I../Rdm/Include \
	-I../libxml/src/include \
	-I$(ETA_ROOT)/Src/Rssl/Include \
	-I$(ETA_ROOT)/Src/ValueAdd/Include 

CFLAGS += -D__EMA_COPY_ON_SET__ -O3 -fbuiltin -Wno-ctor-dtor-privacy -Wno-deprecated -std=c++0x 
	
ifeq ($(findstring Shared,$(LINKTYPE)), Shared)
	OTHER_LIBS = -lxml2 -lrssl -lrsslVA
	CFLAGS += -shared -fPIC

main: libema$(SHARED_SUF)
else
	CFLAGS += -D__EMA_STATIC_BUILD__

main: libema$(STATIC_SUF)
endif

export COMPILER=gnu
export CC=/usr/bin/g++
CFLAGS += -DLinux -DNDEBUG
SYSTEM_LIBS = -lnsl -lpthread -lrt -ldl

# Set flags based on 64 bit machine
CFLAGS += -DCOMPILE_64BITS

$(OUTPUT_DIR)/obj/%.o : %.c
	rm -f $@
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(HEADER_INC) -o $@ $<

$(OUTPUT_DIR)/obj/Access/%.o : %.cpp
	rm -f $@
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(HEADER_INC) -o $@ $<

$(OUTPUT_DIR)/obj/Rdm/%.o : %.cpp
	rm -f $@
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(HEADER_INC) -o $@ $<
	
libaccess_src = \
	Impl/AckMsg.cpp \
	Impl/AckMsgDecoder.cpp \
	Impl/AckMsgEncoder.cpp \
	Impl/Attrib.cpp \
	Impl/ChannelCallbackClient.cpp \
	Impl/ComplexType.cpp \
	Impl/Data.cpp \
	Impl/DataType.cpp \
	Impl/Decoder.cpp \
	Impl/DictionaryCallbackClient.cpp \
	Impl/DirectoryCallbackClient.cpp \
	Impl/ElementEntry.cpp \
	Impl/ElementList.cpp \
	Impl/ElementListDecoder.cpp \
	Impl/ElementListEncoder.cpp \
	Impl/ElementListSetDef.cpp \
	Impl/EmaBuffer.cpp \
	Impl/EmaBufferInt.cpp \
	Impl/EmaBufferU16.cpp \
	Impl/EmaBufferU16Int.cpp \
	Impl/EmaPool.cpp \
	Impl/EmaString.cpp \
	Impl/EmaStringInt.cpp \
	Impl/EncodeIterator.cpp \
	Impl/Encoder.cpp \
	Impl/ExceptionTranslator.cpp \
	Impl/FieldEntry.cpp \
	Impl/FieldList.cpp \
	Impl/FieldListDecoder.cpp \
	Impl/FieldListEncoder.cpp \
	Impl/FieldListSetDef.cpp \
	Impl/FilterEntry.cpp \
	Impl/FilterList.cpp \
	Impl/FilterListDecoder.cpp \
	Impl/FilterListEncoder.cpp \
	Impl/GenericMsg.cpp \
	Impl/GenericMsgDecoder.cpp \
	Impl/GenericMsgEncoder.cpp \
	Impl/HashTable.cpp \
	Impl/ItemCallbackClient.cpp \
	Impl/Key.cpp \
	Impl/LoginCallbackClient.cpp \
	Impl/Map.cpp \
	Impl/MapDecoder.cpp \
	Impl/MapEncoder.cpp \
	Impl/MapEntry.cpp \
	Impl/Msg.cpp \
	Impl/MsgDecoder.cpp \
	Impl/MsgEncoder.cpp \
	Impl/Mutex.cpp \
	Impl/NoDataImpl.cpp \
	Impl/OmmAnsiPage.cpp \
	Impl/OmmAnsiPageDecoder.cpp \
	Impl/OmmAnsiPageEncoder.cpp \
	Impl/OmmArray.cpp \
	Impl/OmmArrayDecoder.cpp \
	Impl/OmmArrayEncoder.cpp \
	Impl/OmmArrayEntry.cpp \
	Impl/OmmAscii.cpp \
	Impl/OmmAsciiDecoder.cpp \
	Impl/OmmBuffer.cpp \
	Impl/OmmBufferDecoder.cpp \
	Impl/OmmConsumerActiveConfig.cpp \
	Impl/OmmConsumerClient.cpp \
	Impl/OmmConsumerConfig.cpp \
	Impl/OmmConsumerConfigImpl.cpp \
	Impl/OmmConsumer.cpp \
	Impl/OmmConsumerEvent.cpp \
	Impl/OmmConsumerImpl.cpp \
	Impl/OmmDate.cpp \
	Impl/OmmDateDecoder.cpp \
	Impl/OmmDateTime.cpp \
	Impl/OmmDateTimeDecoder.cpp \
	Impl/OmmDouble.cpp \
	Impl/OmmDoubleDecoder.cpp \
	Impl/OmmEnum.cpp \
	Impl/OmmEnumDecoder.cpp \
	Impl/OmmError.cpp \
	Impl/OmmErrorDecoder.cpp \
	Impl/OmmException.cpp \
	Impl/OmmFloat.cpp \
	Impl/OmmFloatDecoder.cpp \
	Impl/OmmInaccessibleLogFileException.cpp \
	Impl/OmmInaccessibleLogFileExceptionImpl.cpp \
	Impl/OmmInt.cpp \
	Impl/OmmIntDecoder.cpp \
	Impl/OmmInvalidConfigurationException.cpp \
	Impl/OmmInvalidConfigurationExceptionImpl.cpp \
	Impl/OmmInvalidHandleException.cpp \
	Impl/OmmInvalidHandleExceptionImpl.cpp \
	Impl/OmmInvalidUsageException.cpp \
	Impl/OmmInvalidUsageExceptionImpl.cpp \
	Impl/OmmLoggerClient.cpp \
	Impl/OmmMemoryExhaustionException.cpp \
	Impl/OmmMemoryExhaustionExceptionImpl.cpp \
	Impl/OmmNonRwfEncoder.cpp \
	Impl/OmmOpaque.cpp \
	Impl/OmmOpaqueDecoder.cpp \
	Impl/OmmOpaqueEncoder.cpp \
	Impl/OmmOutOfRangeException.cpp \
	Impl/OmmOutOfRangeExceptionImpl.cpp \
	Impl/OmmQos.cpp \
	Impl/OmmQosDecoder.cpp \
	Impl/OmmReal.cpp \
	Impl/OmmRealDecoder.cpp \
	Impl/OmmRmtes.cpp \
	Impl/OmmRmtesDecoder.cpp \
	Impl/OmmState.cpp \
	Impl/OmmStateDecoder.cpp \
	Impl/OmmSystemException.cpp \
	Impl/OmmSystemExceptionImpl.cpp \
	Impl/OmmTime.cpp \
	Impl/OmmTimeDecoder.cpp \
	Impl/OmmUInt.cpp \
	Impl/OmmUIntDecoder.cpp \
	Impl/OmmUnsupportedDomainTypeException.cpp \
	Impl/OmmUnsupportedDomainTypeExceptionImpl.cpp \
	Impl/OmmUtf8.cpp \
	Impl/OmmUtf8Decoder.cpp \
	Impl/OmmXml.cpp \
	Impl/OmmXmlDecoder.cpp \
	Impl/OmmXmlEncoder.cpp \
	Impl/Payload.cpp \
	Impl/Pipe.cpp \
	Impl/PostMsg.cpp \
	Impl/PostMsgDecoder.cpp \
	Impl/PostMsgEncoder.cpp \
	Impl/RefreshMsg.cpp \
	Impl/RefreshMsgDecoder.cpp \
	Impl/RefreshMsgEncoder.cpp \
	Impl/ReqMsg.cpp \
	Impl/ReqMsgDecoder.cpp \
	Impl/ReqMsgEncoder.cpp \
	Impl/RmtesBuffer.cpp \
	Impl/RmtesBufferImpl.cpp \
	Impl/Series.cpp \
	Impl/SeriesDecoder.cpp \
	Impl/SeriesEncoder.cpp \
	Impl/SeriesEntry.cpp \
	Impl/StaticDecoder.cpp \
	Impl/StatusMsg.cpp \
	Impl/StatusMsgDecoder.cpp \
	Impl/StatusMsgEncoder.cpp \
	Impl/Summary.cpp \
	Impl/Thread.cpp \
	Impl/TimeOut.cpp \
	Impl/UpdateMsg.cpp \
	Impl/UpdateMsgDecoder.cpp \
	Impl/UpdateMsgEncoder.cpp \
	Impl/Utilities.cpp \
	Impl/Vector.cpp \
	Impl/VectorDecoder.cpp \
	Impl/VectorEncoder.cpp \
	Impl/VectorEntry.cpp  
		
librdm_src = \
	../Rdm/Impl/RdmUtilities.cpp

libema_Version_src = \
	EmaVersion.c 

libaccess_objs = $(addprefix $(OUTPUT_DIR)/obj/Access/, $(libaccess_src:%.cpp=%.o))

librdm_objs = $(addprefix $(OUTPUT_DIR)/obj/Rdm/, $(librdm_src:%.cpp=%.o))

libema_Version_objs = $(addprefix $(OUTPUT_DIR)/obj/, $(libema_Version_src:%.c=%.o))

libaccess_static_objs = $(addprefix $(OUTPUT_DIR)/obj/Access/, $(libaccess_src:%.cpp=%.o))

librdm_static_objs = $(addprefix $(OUTPUT_DIR)/obj/Rdm/, $(librdm_src:%.cpp=%.o))

libema_Version_static_objs = $(addprefix $(OUTPUT_DIR)/obj/, $(libema_Version_src:%.c=%.o))

libema$(SHARED_SUF): $(libaccess_objs) $(librdm_objs) $(libema_Version_objs)
	mkdir -p $(OUTPUT_DIR)
	$(CC) -o $(OUTPUT_DIR)/$@ $(libaccess_objs) $(librdm_objs) $(libema_Version_objs) $(CFLAGS) $(HEADER_INC) $(LIB_PATH) $(OTHER_LIBS) $(SYSTEM_LIBS) 

libema$(STATIC_SUF): $(libaccess_static_objs) $(librdm_static_objs) $(libema_Version_static_objs)
	mkdir -p $(OUTPUT_DIR)
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(EMA_LIB_DIR)/libxml2.a
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(UPA_LIB_DIR)/librsslData.a
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(UPA_LIB_DIR)/librsslMessages.a
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(UPA_LIB_DIR)/librsslTransport.a
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(UPA_VA_LIB_DIR)/librsslRDM.a
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(UPA_VA_LIB_DIR)/librsslReactor.a
	cd $(OUTPUT_DIR)/obj; ar -x ../../../../$(UPA_VA_LIB_DIR)/librsslVAUtil.a
	ar -ruv $(OUTPUT_DIR)/$@ $(libaccess_static_objs) $(librdm_static_objs) $(libema_Version_static_objs) $(OUTPUT_DIR)/obj/*.o
	cd $(OUTPUT_DIR)/obj; rm -rf *.o
	
clean:
	rm -rf $(OUTPUT_DIR)
