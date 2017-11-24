#
# GNU Makefile:  TerrainApps/sdlSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/Make.defs

IFLAGS = -I$(X_INC)                \
	 -I/usr/include/SDL        \
	 -I/usr/local/include/SDL  \
	 -I$(TERRAIN_SDK_DIR)      \
	 -I$(VTLIB_DIR)            \
	 -I$(OSG_INC)              \
	 -I$(X_INC)

LDIRS = $(OSG_LIB)                \
	    $(VTDATA_INST_DIR)    \
	    $(VTOSG_INST_DIR)     \
	    $(XMLHELPER_INST_DIR) \
	    $(UNZIP_INST_DIR)     \
	    $(PNG_LIB)            \
	    $(NETCDF_LIB)         \
	    $(X_LIB)              \
	    $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)
EXEC   = sdlSimple
LIBS   = -lvtosg -lvtdata \
         -lgdal -losgDB -losgUtil -losgText -losgParticle -losg \
	 -lxmlhelper -lunzip $(MINI_LIB) \
         `sdl-config --libs` \
         $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure
