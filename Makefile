
ifndef CXX
CXX=g++
endif

ifndef ARIA
ARIA=/usr/local/Aria
endif

ifndef FREENECT2_DIR
FREENECT2_DIR=$(HOME)/libfreenect2.openkinect/examples/protonect
endif

CXXFLAGS+=-fPIC -g -Wall -D_REENTRANT -fno-exceptions
CXXINC=-I$(ARIA)/include -I$(ARIA)/ArNetworking/include -I$(ARIA)/ArVideo/include -I$(FREENECT2_DIR)/include 

LINK_SPECIAL_LIBUSB=-L$(FREENECT2_DIR)/../../depends/libusb/lib -rdynamic -lusb-1.0 -Wl,-rpath,$(FREENECT2_DIR)/../../depends/libusb/lib:$(FREENECT2_DIR)/lib
CXXLINK=-L$(ARIA)/lib -L$(FREENECT2_DIR)/lib -lfreenect2 -lopencv_core -lopencv_core -lopencv_highgui -lturbojpeg -lpthread -lOpenCL $(LINK_SPECIAL_LIBUSB)
#-lArVideo -lArNetworking -lAria -ljpeg -lpthread -ldl -lrt

TARGETS=kinectArVideoServer

all: $(TARGETS)

dist-all: all docs dist-clean


dist-install: install

install: FORCE
	dist/install.sh

clean: 
	rm kinectArVideoServer

docs: doc
doc: cleanDoc
	-mkdir docs
	dist/make-doc.sh

kinectArVideoServer: kinectArVideoServer.cpp ArVideoExternalSource.h ArVideoOpenCV.h
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

FORCE:

.PHONY: docs doc clean install dist-install dist-all all

