# ************** <auto-copyright.pl BEGIN do not edit this line> **************
# Doozer++ is (C) Copyright 2000-2008 by Iowa State University
#
# Original Author:
#   Patrick Hartling
# -----------------------------------------------------------------------------
# VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          dpp.obj.mk,v
# Date modified: 2008/01/01 15:29:22
# Version:       1.15
# -----------------------------------------------------------------
# *************** <auto-copyright.pl END do not edit this line> ***************

# =============================================================================
# dpp.obj.mk,v 1.15 2008/01/01 15:29:22 patrickh Exp
#
# This file <dpp.obj.mk> is intended to be used by makefiles that need to
# build object files.  It defines variables and targets that are common to all
# such makefiles.  It also includes other .mk files that are typically
# necessary for this process.
# -----------------------------------------------------------------------------
# The makefile including this file must define the following variables:
#
# srcdir        - The directory containing the source files to compile.
# OBJEXT        - The suffix used for object files (usually .o or .obj).
# SRCS          - The C++ sources to be compiled.
# JAVA_SRCS     - The Java sources to be compiled.
# MKINSTALLDIRS - Path to shell script for making directories.
# MKPATH        - The path to the .mk files.
#
# Optionally, it can define the following variables for added functionality:
#
# EXTRA_SRCS_PATH  - Directories besides $(srcdir) where source files may be
#                    found.
# OBJDIR           - The directory where the object files will go.  This
#                    defaults to the current directory.
# BEFOREBUILD      - The language-independent target(s) to build before the
#                    object/class files.
# C_BEFOREBUILD    - The C/C++-specific target(s) to build before the object
#                    files.
# JAVA_BEFOREBUILD - The Java-specific target(s) to build before the class
#                    files.
# AFTERBUILD       - The language-independent target(s) to build after the
#                    object/class files.
# C_AFTERBUILD     - The C/C++-specific target(s) to build after the object
#                    files.
# JAVA_AFTERBUILD  - The Java-specific target(s) to build after the class
#                    files.
# TARGETS          - Extra target(s) to build after the object files but
#                    before $(AFTERBUILD).
# JAVA_TARGETS     - Extra target(s) to build after the class files but
#                    before $(AFTERBUILD).
# IDL_CPP_SRCS     - The C++ source files generated by the IDL compiler that
#                    need to be compiled with the other sources.
# IDL_JAVA_SRCS    - The Java source files generated by the Java IDL compiler
#                    that need to be compiled with the other sources.
# -----------------------------------------------------------------------------
# The targets defined here are as follows:
#
# all      - Build everything
# dbg      - Build debugging object files for a static library
# dbg-dso  - Build debugging object files for a dynamic library
# opt      - Build optimized object files for a static library
# opt-dso  - Build optimized object files for a dynamic library
# prof     - Build profiled object files for a static library
# prof-dso - Build profiled object files for a dynamic library
# java     - Build the Java class files
# =============================================================================

include $(MKPATH)/dpp.obj-common.mk

# -----------------------------------------------------------------------------
# Build targets.
# -----------------------------------------------------------------------------
all: dbg dbg-dso opt opt-dso prof prof-dso obj java

dbg dbg-dso opt opt-dso prof prof-dso obj: do-beforebuild
ifdef BEFOREBUILD
	@$(MAKE) $(BEFOREBUILD)
endif
ifdef C_BEFOREBUILD
	@$(MAKE) $(C_BEFOREBUILD)
endif
	@$(MAKE) do-build
ifdef AFTERBUILD
	@$(MAKE) $(AFTERBUILD)
endif
ifdef C_AFTERBUILD
	@$(MAKE) $(C_AFTERBUILD)
endif
ifdef LIB_NAME
	$(AR) $(ARFLAGS) $(LIB_NAME) $(OBJECTS)
endif

java: do-beforebuild
ifdef BEFOREBUILD
	@$(MAKE) $(BEFOREBUILD)
endif
ifdef JAVA_BEFOREBUILD
	@$(MAKE) $(JAVA_BEFOREBUILD)
endif
	@$(MAKE) do-java-build
ifdef AFTERBUILD
	@$(MAKE) $(AFTERBUILD)
endif
ifdef JAVA_AFTERBUILD
	@$(MAKE) $(JAVA_AFTERBUILD)
endif

include $(MKPATH)/dpp.compile.mk
include $(MKPATH)/dpp.dep.mk
include $(MKPATH)/dpp.idl.mk
include $(MKPATH)/dpp.install.mk
include $(MKPATH)/dpp.clean.mk
