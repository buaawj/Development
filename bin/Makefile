##### LIBRARIES CONFIGURATION #####

SYSTEMC := libs/systemc-2.3.1
YAML    := libs/yaml-cpp


##### DIRECTORIES #####

SYSTEMC_LIBS := $(wildcard $(SYSTEMC)/lib-*)

SRCDIR  := ../src
SUBDIRS := $(filter %/,$(wildcard $(SRCDIR)/*/))
OBJDIR  := ./build

SRCS := $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(addsuffix *.cpp,$(SUBDIRS)))
OBJS := $(subst $(SRCDIR),$(OBJDIR),$(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

MODULE := noxim


##### COMPILER OPTIONS #####

CXX      := g++
OPT      := -O3 -MMD
DEBUG    := # -g -DDEBUG
OTHER    := -Wall# -Wno-deprecated

## *************
## Added by LGGM
## *************

## Change the following options to obtain different simulation information
## You might need to perform a make clean after changing any option

# if DEFINE_TM_ASSERT is defined, Noxim stops when abnormal situations occur.
TM_SIM_ASSERT := -DDEFINE_TM_ASSERT
# if DEFINE_TM_WARNING is defined, Noxim shows warnings when they occur.
TM_SIM_WARNING := # -DDEFINE_TM_WARNING
# if DEFINE_TM_SIMCONFIGLOG is defined, tm_engine writes simulation configuration
# logs to the specified log file or the standard output.
TM_SIM_CONFIGLOG := -DDEFINE_TM_SIMCONFIGLOG
# if DEFINE_TM_SIMEXECLOG is defined, Noxim writes simulation execution logs to the
# specified log file.
# if TM_SIMEXECLOG_EXTENDED is defined, Noxim writes extended logs.
TM_SIM_SIMEXECLOG := -DDEFINE_TM_SIMEXECLOG -DTM_SIMEXECLOG_EXTENDED
# if DEFINE_TM_SIMSTATSLOG is defined, tm_engine writes simulation statistics logs
# to the specified log file or the standard output.
TM_SIM_SIMSTATSLOG := -DDEFINE_TM_SIMSTATSLOG
TM_SIM_OPS := $(TM_SIM_ASSERT) $(TM_SIM_WARNING) $(TM_SIM_CONFIGLOG) $(TM_SIM_SIMEXECLOG) $(TM_SIM_SIMSTATSLOG)

CXXFLAGS := $(OPT) $(OTHER) $(DEBUG) $(TM_SIM_OPS)

INCDIR := -I$(SRCDIR) -isystem $(SYSTEMC)/include -I$(YAML)/include
LIBDIR := -L$(SRCDIR) -L$(SYSTEMC_LIBS) -L$(YAML)/lib

LIBS := -lsystemc -lm -lyaml-cpp

SPACE := $(subst ,, )
VPATH := $(SRCDIR):$(subst $(SPACE),:,$(SUBDIRS))

##### RULES #####

$(MODULE): $(OBJS)
	@echo "Linking: $@ ..."
	@$(CXX) $(LIBDIR) $^ $(LIBS) -o $@ 2>&1 | c++filt
	@echo "Process completed."

-include $(DEPS)
$(OBJDIR)/%.o: %.cpp
	@echo "Compiling: $< ..."
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

clean:
	@echo "Removing files ..."
	@rm -rf $(OBJS) $(MODULE) *~ $(OBJDIR)
	@echo "Process completed."
