#Makefile

RM              = rm -f
CXX             = sh4-unknown-linux-gnu-g++
STRIP           = sh4-unknown-linux-gnu-strip
CFLAGS          = -Wall -Wextra -pipe -O2 -DST_OSLINUX -DARCHITECTURE_ST40 -fdata-sections -ffunction-sections
STRIPFLAGS      = --strip-debug --strip-unneeded -g -S
LIBS            = ../../api/lib/plugapi.a -lpthread -lfreetype -lz -lrt
LDFLAGS         = -Wl,--gc-sections -Wl,--strip-all
SRC             = logger.cpp configer.cpp plugin.cpp
OBJ             = $(subst .cpp,.o,$(SRC))

BINNAME         = OSDECMInfo


VPATH           = ../../api/include:../../tools
override CFLAGS += $(patsubst %,-I%,$(subst :, ,$(VPATH)))


all: unix


unix: $(OBJ)
		$(CXX) $(CFLAGS) $(OBJ) $(LIBS) -o $(BINNAME) $(LDFLAGS)
		$(STRIP) $(STRIPFLAGS) $(BINNAME)


plugin.o : plugin.cpp
		$(CXX) $(CFLAGS) plugin.cpp -c -o plugin.o


logger.o : logger.cpp
		$(CXX) $(CFLAGS) ../../tools/logger.cpp -c -o logger.o


configer.o : configer.cpp
		$(CXX) $(CFLAGS) ../../tools/configer.cpp -c -o configer.o


.PHONY : clean install uninstall


clean:
		$(RM) *.o core $(BINNAME)


install: uninstall all
		chmod a+rx $(BINNAME)
		mkdir -p ../../release/$(BINNAME)/plugin
		cp -R ./plugin ../../release/$(BINNAME)
		cp $(BINNAME) ../../release/$(BINNAME)/plugin/bin
		cd ../../release/$(BINNAME); tar -zcvf ../$(BINNAME).tar.gz ./plugin

	
uninstall:
		if test -d ../../release/$(BINNAME)/plugin; then rm -fR ../../release/$(BINNAME); fi
		if test -f ../../release/$(BINNAME).tar.gz; then rm -f ../../release/$(BINNAME).tar.gz; fi

