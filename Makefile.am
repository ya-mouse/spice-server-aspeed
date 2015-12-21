NULL =

AM_CPPFLAGS =					\
	$(GLIB2_CFLAGS)				\
	-I$(top_srcdir)				\
	$(SPICE_PROTOCOL_CFLAGS)		\
	$(NULL)

LDADD =						\
	$(GLIB2_LIBS)				\
	$(SPICE_SERVER_LIBS)			\
	$(NULL)

COMMON_BASE =					\
	basic_event_loop.c			\
	basic_event_loop.h			\
	test_util.h				\
	$(NULL)

sbin_PROGRAMS =				\
	spice_server_aspeed		\
	$(NULL)

spice_server_aspeed_SOURCES =		\
	$(COMMON_BASE)				\
	spice-server-aspeed.c			\
	spice-server-aspeed.h			\
	$(NULL)
