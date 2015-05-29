
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
CXXLINK_OPENCV=-lopencv_core -lopencv_highgui -lopencv_imgproc
CXXLINK_FREENECT=-L$(FREENECT2_DIR)/lib -lfreenect2 -lturbojpeg -lpthread -lOpenCL $(LINK_SPECIAL_LIBUSB) $(CXXLINK_OPENCV)
CXXLINK_ARVIDEO=-L$(ARIA)/lib -lArVideo -lArNetworking -lAria -ljpeg -lpthread -ldl -lrt

#TARGETS+=kinectArVideoServer

all: $(TARGETS)

dist-all: all docs dist-clean


dist-install: install

install: FORCE
	dist/install.sh

clean: 
	-rm kinectArVideoServer testOpenCVServer testRemoteForwarder

docs: doc
doc: cleanDoc
	-mkdir docs
	dist/make-doc.sh

kinectArVideoServer: kinectArVideoServer.cpp ArVideoExternalSource.h ArVideoOpenCV.h
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK_FREENECT) $(CXXLINK_ARVIDEO)

testOpenCVServer: testOpenCVServer.cpp ArVideoExternalSource.h ArVideoOpenCV.h ArCallbackList4.h
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK_OPENCV) $(CXXLINK_ARVIDEO)

testRemoteForwarder: testRemoteForwarder.cpp ArVideoExternalSource.h ArVideoOpenCV.h ArVideoRemoteSource.h ArVideoRemoteSource.cpp ArCallbackList4.h
	$(CXX) $(CXXFLAGS) $(CXXINC) $< ArVideoRemoteSource.cpp -o $@ $(CXXLINK_ARVIDEO)

FORCE:

.PHONY: docs doc clean install dist-install dist-all all

