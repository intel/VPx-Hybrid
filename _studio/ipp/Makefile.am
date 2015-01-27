AUTOMAKE_OPTIONS=foreign

INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/_studio/ipp/include

ipp_sources = src/ipp.c

noinst_LTLIBRARIES = libipp-glibc.la

libipp_glibc_la_SOURCES = $(ipp_sources)
libipp_glibc_la_LIBADD = @LIBOBJS@
