#############################################################
#		Universal Makefile process for GNU make.
#
# License:		MIT
# Author:		Sloong.com
# UpdateDate:	20180308
#
#===========================================================================

## Customizable Section: adapt those variables to suit your program.
##==========================================================================
# The executable file name.
# If not specified, current directory name or `a.out' will be used.
PROGRAM   =  sloongnet_mysql.so

#Type
# so  -> share library
# ar  -> static library
# app -> Application
PROJECT_TYPE = so

# The pre-processor and compiler options.
MY_CFLAGS =

# Warning flag
WARNINGS := -Wall

# debug flag
DEBUGFLAGS = -DDEBUG -g -pg 

# release flag
RELEASEFLAGS = -DNODEBUG -DRELASE -O3

# The linker options.
LIBS   = -lmysqlclient -luniv 

# include directory
INCLUDE_DIR = 

# The options used in linking as well as in any direct use of ld.
LDFLAGS   = -L/usr/local/lib/sloong -Wl,-rpath=/usr/local/lib/sloong -L/usr/lib64/mysql -Wl,-rpath=/usr/lib64/mysql

# The directories in which source files reside.
# If not specified, only the current directory will be serached.
SRCDIRS	=

## Implicit Section: change the following only when necessary.
##==========================================================================

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .c .C .cc .cpp .CPP .c++ .cxx .cp

# The header file types.
HDREXTS = .h .H .hh .hpp .HPP .h++ .hxx .hp

# The pre-processor and compiler options.
# Users can override those variables from the command line.
CFLAGS = -g -pthread
CXXFLAG= -std=c++17 -DLUA_USE_READLINE 

# The C program compiler.
CC	 = gcc

# The C++ program compiler.
CXX	= g++

# Un-comment the following line to compile C programs as C++ ones.
CC	 = $(CXX)

# The command used to delete file.
RM	 = rm -f

ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
# check the project type 
ifeq ($(filter $(PROJECT_TYPE),so ar app),)
$(error Unexpected PROJECT_TYPE `$(PROJECT_TYPE)')
endif

ifeq ($(PROJECT_TYPE),so)
 CFLAGS  += -fPIC
 CXXFLAG += -fPIC
 LDFLAGS += -shared
endif

SHELL   = /bin/sh
EMPTY   =
SPACE   = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
  CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
  PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
  ifeq ($(PROGRAM),)
	PROGRAM = a.out
  endif
endif
ifeq ($(SRCDIRS),)
  SRCDIRS = .
endif
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS	= $(addsuffix .o, $(basename $(SOURCES)))
#DEPS	= $(OBJS:.o=.d)
## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then echo "-MM -MP"; else echo "-M"; fi )
DEPEND = $(CC) $(DEP_OPT)  $(MY_CFLAGS) $(CFLAGS) 
DEPEND.d = $(subst -g ,,$(DEPEND))
COMPILE.c = $(CC)  $(MY_CFLAGS) $(CFLAGS) $(INCLUDE_DIR) $(WARNINGS) -c
COMPILE.cxx = $(CXX) $(MY_CFLAGS)  $(CXXFLAGS) $(INCLUDE_DIR) $(WARNINGS) -c
LINK.c = $(CC)  $(MY_CFLAGS) $(CFLAGS) $(LDFLAGS)
LINK.cxx = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(LDFLAGS)

.PHONY: all objs tags ctags clean distclean help show

# Delete the default suffixes
.SUFFIXES:

all: Release

Debug : CXXFLAGS = $(DEBUGFLAGS) $(CXXFLAG)
Debug :	$(PROGRAM)

debug : Debug

Release : CXXFLAGS = $(RELEASEFLAGS) $(CXXFLAG)
Release: $(PROGRAM)

release : Release

# Rules for creating dependency files (.d).
#------------------------------------------

%.d:%.c
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.C
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cc
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cpp
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.CPP
	echo -n $(dir $<) > $@
	$(DEPEND.d) $< >> $@

%.d:%.c++
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cp
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.cxx
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

%.o:%.c
	$(COMPILE.c) $< -o $@

%.o:%.C
	$(COMPILE.cxx) $< -o $@

%.o:%.cc
	$(COMPILE.cxx) $< -o $@

%.o:%.cpp
	$(COMPILE.cxx) $< -o $@

%.o:%.CPP
	$(COMPILE.cxx) $< -o $@

%.o:%.c++
	$(COMPILE.cxx) $< -o $@

%.o:%.cp
	$(COMPILE.cxx) $< -o $@

%.o:%.cxx
	$(COMPILE.cxx) $< -o $@

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

# Rules for generating the executable.
#-------------------------------------
$(PROGRAM):$(OBJS)
ifeq ($(SRC_CXX),)			  # C program
	$(LINK.c)   $(OBJS) $(LIBS) -o $@
	@echo Type ./$@ to execute the program.
else							# C++ program
	$(LINK.cxx) $(OBJS) $(LIBS) -o $@
	@echo Type ./$@ to execute the program.
endif

ifndef NODEP
ifneq ($(DEPS),)
  sinclude $(DEPS)
endif
endif

clean:
	$(RM) $(OBJS) $(PROGRAM)

distclean: clean
	$(RM) $(DEPS) TAGS


## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
##############################################################
