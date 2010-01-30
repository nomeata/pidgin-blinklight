# EXTRA_DIST = 

gtdir = $(PURPLE_LIBDIR)/purple-2

pidgin_blinklight_la_LDFLAGS = -module -avoid-version

if PLUGINS

gt_LTLIBRARIES = pidgin-blinklight.la

pidgin_blinklight_la_SOURCES = \
	pidgin-blinklight.c blink.c

pidgin_blinklight_la_CPPFLAGS = \
	-DVERSION=\"$(VERSION)\" \
	-DLIBDIR=\"$(PURPLE_LIBDIR)/purple-2/\" \
	-DDATADIR=\"$(PURPLE_DATADIR)\" \
	-DLOCALEDIR=\"$(PURPLE_DATADIR)/locale\" \
	$(DEBUG_CFLAGS) \
	$(PURPLE_CFLAGS)

endif

libexec_PROGRAMS = blinklight-fixperm

install-exec-hook:
	chmod +s $(DESTDIR)$(libexecdir)/blinklight-fixperm$(EXEEXT)
