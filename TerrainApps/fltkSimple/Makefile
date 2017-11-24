#
# GNU Makefile:  TerrainApps/fltkSimple
#

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = fltkSimple
include $(VTP_ROOT)/Make.defs

IFLAGS = -I$(X_INC)                 \
         -I$(TERRAIN_SDK_DIR)       \
         -I$(VTLIB_DIR)             \
	 -I$(OSG_INC)               \
	 -I$(GDAL_INC)              \
	 -I$(X_INC)

LDIRS  = $(OSG_LIB)             \
	 $(VTDATA_INST_DIR)     \
	 $(XMLHELPER_INST_DIR)  \
	 $(UNZIP_INST_DIR)      \
	 $(VTOSG_INST_DIR)      \
         $(PNG_LIB)             \
         $(NETCDF_LIB)          \
         $(GDAL_LIB)            \
	 $(X_LIB)               \
	 $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) $(EXTRA_LDFLAGS) $(shell fltk-config --use-gl --ldflags)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)

ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   = -bind_at_load -F$(FRAMEWORKS) \
	$(call _VLF,vtosg) $(call _VLF,vtdata) $(call _VLF,xmlhelper) $(call _VLF,unzip) \
	$(OSG_OPTS) \
	-lgdal -losgGA -losgDB -losgUtil -losgText -losg -lOpenThreads \
	-L$(FINK_LIB) -lnetcdf \
	$(MINI_LIB) \
        $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) \
	-framework Carbon -lobjc -lm 

all : $(OSX_APP)

else
#LIBS   = $(shell fltk-config --use-gl --libs)
LIBS   = -lvtosg -lvtdata \
         -lgdal -losgDB -losgUtil -losgText -losgParticle -losg \
	 -lxmlhelper -lunzip $(MINI_LIB) \
         $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)
endif



$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

$(OSX_APP) : $(OBJS) $(OSX_APPS)/$(EXEC).app
	$(LD) -o $(OSX_APP) $(OBJS) $(LDFLAGS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure $(OSX_APPS)/$(EXEC).app
