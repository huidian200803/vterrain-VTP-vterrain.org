#
# GNU Makefile:  TerrainApps/prSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = prSimple
include $(VTP_ROOT)/Make.defs

IFLAGS = -I$(X_INC)                 \
         -I$(TERRAIN_SDK_DIR)       \
         -I$(VTLIB_DIR)             \
	 -I$(OSG_INC)               \
	 -I$(GDAL_INC)              \
	 -I$(X_INC)

LDIRS  = $(OSG_LIB)                     \
	 $(VTDATA_INST_DIR)             \
	 $(XMLHELPER_INST_DIR)          \
	 $(XMLHELPER_INST_DIR)          \
	 $(VTOSG_INST_DIR)              \
         $(PNG_LIB)                     \
         $(NETCDF_LIB)                  \
         $(GDAL_LIB)                    \
	 $(X_LIB)                       \
	 $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1 -D__USE_OSX_AGL_IMPLEMENTATION__
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)

ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   = -bind_at_load -F$(FRAMEWORKS) \
	-framework vtosg -framework vtdata \
	-lgdal \
	-framework osg -framework osgUtil \
    -framework osgText -framework osgDB -framework OpenThreads -framework Producer \
	-L$(FINK_LIB) -lnetcdf \
	-framework xmlhelper -framework unzip $(MINI_LIB) \
        $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) \
	-framework Carbon -lobjc -lm \
	-Wl,-executable_path -Wl,$(OSX_APPS)/$(EXEC).app/Contents/MacOS

# Use these instead if you are not using osg frameworks:
# -losgDB -losgUtil -losgText -losg -lOpenThreads -lProducer

all : $(OSX_APP)

else
LIBS   = -lvtosg -lvtdata \
         -lgdal -losgDB -losgUtil -losgText -losg -lProducer -lOpenThreads -lnetcdf \
		 -lxmlhelper -lunzip $(MINI_LIB) \
         $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lcurl -lbz2 -lm 

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
