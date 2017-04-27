include ../Common/makefile.common

optimizedStatic:	OUTPUT_DIR = $(IDENT)/Optimized
optimizedStatic: outputDir
	make OUTPUT_DIR=$(OUTPUT_DIR) libPerfTools.a

optimizedShared:	OUTPUT_DIR = $(IDENT)/Optimized/Shared
optimizedShared: outputDir
	make OUTPUT_DIR=$(OUTPUT_DIR) SHARED_CFLAG="-fPIC" libPerfTools.so

optimizedAssertStatic:	OUTPUT_DIR = $(IDENT)/Optimized_Assert
optimizedAssertStatic: outputDir
	make OUTPUT_DIR=$(OUTPUT_DIR) libPerfTools.a

optimizedAssertShared:	OUTPUT_DIR = $(IDENT)/Optimized_Assert/Shared
optimizedAssertShared: outputDir
	make OUTPUT_DIR=$(OUTPUT_DIR) SHARED_CFLAG="-fPIC" libPerfTools.so

.PHONY:  all
all:
	$(foreach T, optimizedStatic optimizedShared optimizedAssertStatic optimizedAssertShared, $(MAKE) $T;)

outputDir:
	@mkdir -p $(OUTPUT_DIR)/obj

clean:
	/bin/rm -fr $(IDENT)

SRC=	$(shell ls *.cpp)
OBJS=   $(SRC:%.cpp=$(OUTPUT_DIR)/obj/%.o)

libPerfTools.a:   $(OBJS)
	ar rvs $(OUTPUT_DIR)/obj/$@ $(OBJS)
libPerfTools.so:   $(OBJS)
	g++ -shared -o $(OUTPUT_DIR)/obj/$@ $(OBJS)

$(OUTPUT_DIR)/obj/%.o: %.cpp
	$(CC) $(CFLAGS) $(SHARED_CFLAG) $(INCLUDES) -c $< -o $@


