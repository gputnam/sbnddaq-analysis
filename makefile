#Makefile for gallery c++ programs.
#Note, being all-incllusive here: you can cut out libraries/includes you do not need
#you can also change the flags if you want too (Werror, pedantic, etc.)

CPPFLAGS=-I $(BOOST_INC) \
         -I $(CANVAS_INC) \
         -I $(CETLIB_INC) \
         -I $(CETLIB_EXCEPT_INC) \
         -I $(FHICLCPP_INC) \
         -I $(GALLERY_INC) \
         -I $(LARCOREOBJ_INC) \
         -I $(LARDATAOBJ_INC) \
         -I $(NUSIMDATA_INC) \
         -I $(ROOT_INC) \
	 -I $(TRACE_INC) \
	 -I $(ARTDAQ_CORE_INC) \
         -I $(FFTW_INC) \
	 -I $(SBNDDAQ_DATATYPES_INC)
	 #-I $(BERNFEBDAQ_CORE_INC)

CXXFLAGS=-std=c++14 -Wall -Werror -pedantic
CXX=g++
LDFLAGS=$$(root-config --libs) \
        -L $(CANVAS_LIB) -l canvas \
        -L $(CETLIB_LIB) -l cetlib \
        -L $(CETLIB_EXCEPT_LIB) -l cetlib_except \
        -L $(GALLERY_LIB) -l gallery \
        -L $(NUSIMDATA_LIB) -l nusimdata_SimulationBase \
        -L $(LARCOREOBJ_LIB) -l larcoreobj_SummaryData \
        -L $(LARDATAOBJ_LIB) -l lardataobj_RecoBase -l lardataobj_MCBase -l lardataobj_RawData -l lardataobj_OpticalDetectorData -l lardataobj_AnalysisBase \
	-L $(SBNDDAQ_DATATYPES_LIB) -l sbnddaq-datatypes_Overlays -l sbnddaq-datatypes_NevisTPC \
        -L $(FFTW_LIBRARY) -l fftw3
	#-L $(ARTDAQ_CORE_INC) -l artdaq_core
	#-L $(BERNFEBDAQ_CORE_LIB) -l bernfebdaq_core_Overlays

EXEC=analysis
OBJECTS = Main.o FFT.o
SOURCES = $(OBJECTS:.o=.cc)

$(EXEC): $(OBJECTS)
	@echo Building $(EXEC)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

%.o : %.cxx
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm $(EXEC) $(OBJECTS)

all: analysis
