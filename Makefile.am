MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop

SUBDIRS = src

EDJE_FLAGS = -v \
	   -id $(top_srcdir)/images \
	   -fd $(top_srcdir)/fonts

# data files for the module
filesdir =  $(datadir)

files_DATA = module.desktop \
	     e-module-dbus-api.edj

EXTRA_DIST = $(files_DATA) \
	     images/module_icon.png \
	     fonts/VeraBd.ttf

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-dbus-api.edj module.desktop *~
uninstall:
	 rm -rf $(DESTDIR)$(datadir)
	 
