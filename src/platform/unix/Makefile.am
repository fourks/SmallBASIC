# SmallBASIC command line version
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = -I../ -I. @PACKAGE_CFLAGS@

EXTRA_DIST =																		\
    chr2c.c																			\
    el-928.c																		\
    lnx928.fnt																	\
    mgr928.fnt																	\
    mod928.fnt																	\
    mugr928.fnt																	\
    nar928-2.fnt																\
    nar928.fnt																	\
    ndc928.c																		\
    ndc928.fnt																	\
    rlgr928.fnt																	\
    rom16.c																			\
    inet.c

bin_PROGRAMS = sbasic

sbasic_SOURCES =																\
		dev_ndcfb.c																	\
	  dev_genfb.c																	\
		device.c																		\
		dev_oss.c																		\
		dev_term.c																	\
	  console_main.c															\
    ../../common/g_line.c												\
	  ../../common/decomp.c

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/src -lsb_common

if !WITH_WIN32
sbasic_LDADD += -ldl
endif

sbasic_DEPENDENCIES = ../libsb_common.a