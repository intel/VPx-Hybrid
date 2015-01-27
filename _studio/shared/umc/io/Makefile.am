AUTOMAKE_OPTIONS=foreign

INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/_studio/ipp/include \
  -I$(top_srcdir)/_studio/shared/include \
  -I$(top_srcdir)/_studio/shared/umc/core/umc/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm_plus/include \
  -I$(top_srcdir)/_studio/shared/umc/io/umc_va/include

umc_va_sources = \
  umc_va/src/umc_va.cpp \
  umc_va/src/umc_va_linux.cpp

noinst_LTLIBRARIES = \
  libumc_io_merged.la

libumc_io_merged_la_SOURCES = \
  $(umc_io_sources)
libumc_io_merged_la_LIBADD = @LIBOBJS@
libumc_io_merged_la_CXXFLAGS = $(MFX_CFLAGS_INTERNAL)

noinst_LTLIBRARIES += libumc_va.la

libumc_va_la_SOURCES = \
  $(umc_va_sources)
libumc_va_la_LIBADD = @LIBOBJS@
libumc_va_la_CXXFLAGS = \
  $(MFX_CFLAGS_INTERNAL) \
  $(LIBVA_CFLAGS) \
  -DMFX_VA
