AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I$(top_builddir)/sharelib

EXTRA_DIST = 

bin_PROGRAMS = octopustool octopusd 

octopusd_SOURCES = octopusd.cpp octopusd.h
octopusd_DEPENDENCIES = $(top_builddir)/sharelib/liboctopusd.a octopusd.h
if WITHMYSQLCONFIG
octopusd_LDADD = $(top_builddir)/sharelib/liboctopusd.a -lmysqlclient -lpthread
else
octopusd_LDADD = $(top_builddir)/sharelib/liboctopusd.a -lpthread
endif

octopustool_SOURCES = octopustool.cpp
octopustool_DEPENDENCIES = $(top_builddir)/sharelib/liboctopusd.a
if WITHMYSQLCONFIG
octopustool_LDADD = $(top_builddir)/sharelib/liboctopusd.a -lmysqlclient -lpthread
else
octopustool_LDADD = $(top_builddir)/sharelib/liboctopusd.a -lpthread
endif

#install-exec-hook:
#	# ln $(DESTDIR)$(bindir)/program$(EXEEXT) \
#	#        $(DESTDIR)$(bindir)/proglink$(EXEEXT)

