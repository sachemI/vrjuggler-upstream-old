# -----------------------------------------------------------------------------
# $Id$
# this file ripped off from patrick, that makefile genius...
#
# This include file handles source code dependencies.  It
# generates makefiles (.d files actually) corresponding to each source file
# that have the dependencies for that source file.  The following sorce file
# types are supported: .c, .C, .CC, .cc, .cxx, .cpp and .c++.
# -----------------------------------------------------------------------------
# The makefile that includes this file must define the following variables:
#
# CC          - The C compiler.
# C_COMPILE   - The compiler command for C files.
# CXX_COMPILE - The compiler command for C++ files.
# OBJDIR      - The directory to which the object file(s) will be written.
# OBJECT_EXT  - Suffix for object file names (e.g., "o" or "obj").
# OBJS        - The list of object files that will be compiled.  We use this
#               for the suffix change because there is only one suffix for
#               object files ($(OBJEXT)).
# -----------------------------------------------------------------------------

DEPEND_FILES	= $(OBJS:.$(OBJEXT)=.d)
OBJDIR ?= .

# These expressions reformat the output from the dependency text to be of the
# form:
#
#     $(OBJDIR)/file1.o file1.d : ...
#
# where file1 is the value in $* and file1.d is $@.  The first handles output
# from the C and C++ compilers which prints only the object file to be
# created ($*).  The second handles output from makedepend(1) which includes
# the path leading to the source file in the object file name.
_CC_SED_EXP	= '\''s/\($*\)\.$(OBJEXT)[ :]*/$$(OBJDIR)\/\1.$(OBJEXT) $@: /g'\''
_MKDEP_SED_EXP	= '\''s/.*\($*\)\.$(OBJEXT)[ :]*/$$(OBJDIR)\/\1.$(OBJEXT) $@: /g'\''

# Determine, based on the C compiler, how to generate the dependencies.  If
# the compiler is cl (Windows only), use makedepend(1).  Otherwise, we can use
# the compiler itself to do the job.
ifeq ($(USE_MAKEDEPEND), Y)
    _C_DEPGEN	= $(SHELL) -ec 'makedepend -f- -o.$(OBJEXT)    \
                      $(DEPENDFLAGS) -- $(DEPEND_EXTRAS) -- $< |	\
                      sed $(_MKDEP_SED_EXP) > $@ ; [ -s $@ ] || rm -f $@'
    _CXX_DEPGEN	= $(SHELL) -ec 'makedepend -f- -o.$(OBJEXT)	\
                      $(DEPENDFLAGS) -- $(DEPEND_EXTRAS) -- $< |	\
                      sed $(_MKDEP_SED_EXP) > $@ ; [ -s $@ ] || rm -f $@'
else
    _C_DEPGEN	= $(SHELL) -ec '$(C_COMPILE) $(INCLUDE_PATHS) $(DEP_GEN_FLAG) $< | \
                      sed $(_CC_SED_EXP) > $@ ; [ -s $@ ] || rm -f $@'
    _CXX_DEPGEN	= $(SHELL) -ec '$(CXX_COMPILE) $(INCLUDE_PATHS) $(DEP_GEN_FLAG) $< | \
                      sed $(_CC_SED_EXP) > $@ ; [ -s $@ ] || rm -f $@'
endif

%.d: %.c
	@echo "Updating dependency file $@ ..."
	@$(_C_DEPGEN)

%.d: %.C
	@echo "Updating dependency file $@ ..."
	@$(_CXX_DEPGEN)

%.d: %.CC
	@echo "Updating dependency file $@ ..."
	@$(_CXX_DEPGEN)

%.d: %.cc
	@echo "Updating dependency file $@ ..."
	@$(_CXX_DEPGEN)

%.d: %.c++
	@echo "Updating dependency file $@ ..."
	@$(_CXX_DEPGEN)

%.d: %.cpp
	@echo "Updating dependency file $@ ..."
	@$(_CXX_DEPGEN)

%.d: %.cxx
	@echo "Updating dependency file $@ ..."
	@$(_CXX_DEPGEN)

# Include all the files in $(DEPEND_FILES) unless we are cleaning up the
# dependencies.  There are other cases when we do not want to include all
# these files, but I do not know of a good way to do the conditional include
# besides nested, gross conditional statements.
ifndef DO_CLEANDEPEND
    include $(DEPEND_FILES)
endif
