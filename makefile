#Makefile for gallery c++ programs.
#Note, being all-incllusive here: you can cut out libraries/includes you do not need
#you can also change the flags if you want too (Werror, pedantic, etc.)

# defining locations
SRC=src
BUILD=$(SBNDDAQ_ANALYSIS_BUILD_PATH)

# and filenames
SRCEXT=cc
OBJEXT=o

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
	 -I $(SBNDDAQ_DATATYPES_INC) \
         -I $(JSONCPP_INC)
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
        -L $(FFTW_LIBRARY) -l fftw3 \
        -L $(JSONCPP_LIB) -l jsoncpp \
        -L $(BUILD) -l sbnddaq_analysis_data_dict \
	#-L $(ARTDAQ_CORE_INC) -l artdaq_core
	#-L $(BERNFEBDAQ_CORE_LIB) -l bernfebdaq_core_Overlays


# executable
EXEC=analysis
SOURCES=$(shell find $(SRC) -type f -name *.$(SRCEXT))
OBJECTS=$(patsubst $(SRC)/%,$(BUILD)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))
#OBJECTS+=dict

# commands
all: dict analysis
dict: 
	@rootcint -f libsbnddaq_analysis_data_dict.cxx $(SRC)/ChannelData.hh $(SRC)/HeaderData.hh $(SRC)/Noise.hh $(SRC)/Noise.cc $(SRC)/FFT.hh $(SRC)/FFT.cc $(SRC)/linkdef.h
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -shared -fPIC -o $(BUILD)/libsbnddaq_analysis_data_dict.so libsbnddaq_analysis_data_dict.cxx
	@mv libsbnddaq_analysis_data_dict* $(BUILD)/
$(EXEC): $(OBJECTS)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
$(BUILD)/%.$(OBJEXT): $(SRC)/%.$(SRCEXT)
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $< -o $@
check:
	@echo $(OBJECTS)
	@echo $(SOURCES)
	@echo $(BUILD)
clean:
	rm $(EXEC) $(OBJECTS) $(BUILD)/libsbnddaq_analysis_data_dict*
