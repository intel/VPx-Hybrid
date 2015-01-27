AUTOMAKE_OPTIONS=foreign

INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/_studio/ipp/include \
  -I$(top_srcdir)/_studio/mfx_lib/shared/include \
  -I$(top_srcdir)/_studio/shared/include \
  -I$(top_srcdir)/_studio/shared/umc/core/umc/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm_plus/include \
  -I$(top_srcdir)/_studio/shared/umc/io/umc_va/include

scheduler_includes = \
  -I$(top_srcdir)/_studio/mfx_lib/scheduler/include

MFX_LIB_INCLUDES = \
  $(scheduler_includes)

INCLUDES += \
  $(MFX_LIB_INCLUDES)

# sources which will fall to the libumc_codec_common_merged
scheduler_sources = \
  scheduler/src/mfx_scheduler_core.cpp \
  scheduler/src/mfx_scheduler_core_ischeduler.cpp \
  scheduler/src/mfx_scheduler_core_iunknown.cpp \
  scheduler/src/mfx_scheduler_core_task.cpp \
  scheduler/src/mfx_scheduler_core_task_management.cpp \
  scheduler/src/mfx_scheduler_core_thread.cpp

shared_sources = \
  shared/src/mfx_common_int.cpp

shared_hw_sources = \
  $(shared_sources)

mfx_lib_hw_sources = \
  $(scheduler_sources) \
  $(shared_hw_sources)

so_sources = \
  shared/src/libmfxsw.cpp \
  shared/src/libmfxsw_async.cpp \
  shared/src/libmfxsw_decode.cpp \
  shared/src/libmfxsw_enc.cpp \
  shared/src/libmfxsw_encode.cpp \
  shared/src/libmfxsw_plugin.cpp \
  shared/src/libmfxsw_query.cpp \
  shared/src/libmfxsw_session.cpp \
  shared/src/libmfxsw_vpp.cpp \
  shared/src/mfx_session.cpp \
  shared/src/mfx_user_plugin.cpp \
  $(srcdir)/../shared/src/fast_copy.cpp \
  $(srcdir)/../shared/src/libmfx_allocator.cpp \
  $(srcdir)/../shared/src/libmfx_allocator_vaapi.cpp \
  $(srcdir)/../shared/src/libmfx_core.cpp \
  $(srcdir)/../shared/src/libmfx_core_factory.cpp \
  $(srcdir)/../shared/src/libmfx_core_vaapi.cpp \
  $(srcdir)/../shared/src/mfx_umc_alloc_wrapper.cpp

noinst_LTLIBRARIES =
lib_LTLIBRARIES =

noinst_LTLIBRARIES += libmfx_lib_merged_hw.la

libmfx_lib_merged_hw_la_SOURCES = \
  $(mfx_lib_hw_sources)
libmfx_lib_merged_hw_la_LIBADD = @LIBOBJS@
libmfx_lib_merged_hw_la_CFLAGS = \
  $(MFX_CFLAGS_INTERNAL) \
  $(LIBVA_CFLAGS) \
  -DMFX_RT \
  -DMFX_VA
libmfx_lib_merged_hw_la_CXXFLAGS = \
  $(MFX_CFLAGS_INTERNAL) \
  $(LIBVA_CFLAGS) \
  -DMFX_RT \
  -DMFX_VA

if BUILD_32
  lib_LTLIBRARIES += libmfxhw32.la

  libmfxhw32_la_SOURCES = \
    $(so_sources)
  libmfxhw32_la_DEPENDENCIES = \
    libmfx_lib_merged_hw.la \
    $(top_builddir)/_studio/shared/umc/io/libumc_va.la \
    $(top_builddir)/_studio/shared/umc/io/libumc_io_merged.la \
    $(top_builddir)/_studio/shared/umc/core/libumc_core_merged.la \
    $(top_builddir)/_studio/ipp/libipp-glibc.la
  libmfxhw32_la_LIBADD = \
    $(libmfxhw32_la_DEPENDENCIES) \
    $(LIBVA_LIBS) \
    -ldl -lpthread
  libmfxhw32_la_CFLAGS = \
    $(MFX_CFLAGS_INTERNAL) \
    $(LIBVA_CFLAGS) \
    -DMFX_VA
  libmfxhw32_la_CXXFLAGS = \
    $(MFX_CFLAGS_INTERNAL) \
    $(LIBVA_CFLAGS) \
    -DMFX_VA
  libmfxhw32_la_LDFLAGS = \
    -Wl,--version-script=$(srcdir)/libmfx.map \
    $(MFX_LDFLAGS) \
    $(MEDIASDK_LT_LDFLAGS)
endif
if BUILD_64
  lib_LTLIBRARIES += libmfxhw64.la

  libmfxhw64_la_SOURCES = \
    $(so_sources)
  libmfxhw64_la_DEPENDENCIES = \
    libmfx_lib_merged_hw.la \
    $(top_builddir)/_studio/shared/umc/io/libumc_va.la \
    $(top_builddir)/_studio/shared/umc/io/libumc_io_merged.la \
    $(top_builddir)/_studio/shared/umc/core/libumc_core_merged.la \
    $(top_builddir)/_studio/ipp/libipp-glibc.la
  libmfxhw64_la_LIBADD = \
    $(libmfxhw64_la_DEPENDENCIES) \
    $(LIBVA_LIBS) \
    -ldl -lpthread
  libmfxhw64_la_CFLAGS = \
    $(MFX_CFLAGS_INTERNAL) \
    $(LIBVA_CFLAGS) \
    -DMFX_VA \
    -DMFX_RT
  libmfxhw64_la_CXXFLAGS = \
    $(MFX_CFLAGS_INTERNAL) \
    $(LIBVA_CFLAGS) \
    -DMFX_VA \
    -DMFX_RT
  libmfxhw64_la_LDFLAGS = \
    -Wl,--version-script=$(srcdir)/libmfx.map \
    $(MFX_LDFLAGS) \
    $(MEDIASDK_LT_LDFLAGS)
endif
