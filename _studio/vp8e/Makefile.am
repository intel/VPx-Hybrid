AUTOMAKE_OPTIONS=foreign

INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/_studio/ipp/include \
  -I$(top_srcdir)/_studio/mfx_lib/shared/include \
  -I$(top_srcdir)/_studio/shared/include \
  -I$(top_srcdir)/_studio/shared/umc/core/umc/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm_plus/include \
  -I$(top_srcdir)/_studio/vp8e/vp8e_plugin/include

bse_sources = vp8e_plugin/src/mfx_vp8_encode_hybrid_bse.cpp

so_sources = \
  vp8e_plugin/src/mfx_vp8_enc_common_hw.cpp \
  vp8e_plugin/src/mfx_vp8_encode_ddi_hw.cpp \
  vp8e_plugin/src/mfx_vp8_encode_utils_hybrid_hw.cpp \
  vp8e_plugin/src/vp8e_plugin.cpp \
  $(srcdir)/../mfx_lib/shared/src/mfx_enc_common.cpp \
  $(srcdir)/../mfx_lib/shared/src/mfx_common_int.cpp \
  $(srcdir)/../shared/umc/core/vm_plus/src/mfx_vm++_pthread.cpp \
  $(srcdir)/../shared/umc/core/vm_plus/src/mfx_vm++_pthread_unix.cpp \
  $(srcdir)/../shared/umc/core/vm/src/vm_mutex_linux32.c

noinst_LTLIBRARIES =
lib_LTLIBRARIES =

if ENABLE_PRIVATE_BUILD
if BUILD_32
    lib_LTLIBRARIES += libmfx_vp8e_bse_32.a

    libmfx_vp8e_bse_32_a_SOURCES = \
      $(bse_sources)
    libmfx_vp8e_bse_32_a_CFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_bse_32_a_CXXFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_bse_32_a_LDFLAGS = \
      $(MFX_LDFLAGS) \
      $(MEDIASDK_LT_LDFLAGS)

endif
if BUILD_64
    lib_LTLIBRARIES += libmfx_vp8e_bse_64.a

    libmfx_vp8e_bse_64_a_SOURCES = \
      $(bse_sources)
    libmfx_vp8e_bse_64_a_CFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_bse_64_a_CXXFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_bse_64_a_LDFLAGS = \
      $(MFX_LDFLAGS) \
      $(MEDIASDK_LT_LDFLAGS)

endif
endif

if BUILD_32
    lib_LTLIBRARIES += libmfx_vp8e_plugin_32.la

    libmfx_vp8e_plugin_32_la_SOURCES = \
      $(so_sources)
if ENABLE_PRIVATE_BUILD
    libmfx_vp8e_plugin_32_la_DEPENDENCIES = \
      libmfx_vp8e_bse_32.a
    VP8E_BSE_LIB =
else
    libmfx_vp8e_plugin_32_la_DEPENDENCIES =
    VP8E_BSE_LIB=vp8e_plugin/lib/libmfx_vp8e_bse_32.a
endif
    libmfx_vp8e_plugin_32_la_DEPENDENCIES += \
      $(top_builddir)/_studio/ipp/libipp-glibc.la
    libmfx_vp8e_plugin_32_la_LIBADD = \
      $(libmfx_vp8e_plugin_32_la_DEPENDENCIES) \
      $(VP8E_BSE_LIB) \
      $(LIBVA_LIBS) \
      -lpthread
    libmfx_vp8e_plugin_32_la_CFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      $(LIBVA_CFLAGS) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_plugin_32_la_CXXFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      $(LIBVA_CFLAGS) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_plugin_32_la_LDFLAGS = \
      -Wl,--version-script=$(srcdir)/libmfx_vp8e_plugin.map \
      $(MFX_LDFLAGS) \
      $(MEDIASDK_LT_LDFLAGS)
endif
if BUILD_64
    lib_LTLIBRARIES += libmfx_vp8e_plugin_64.la

    libmfx_vp8e_plugin_64_la_SOURCES = \
      $(so_sources)
if ENABLE_PRIVATE_BUILD
    libmfx_vp8e_plugin_64_la_DEPENDENCIES = \
      libmfx_vp8e_bse_64.a
    VP8E_BSE_LIB=
else
    libmfx_vp8e_plugin_64_la_DEPENDENCIES =
    VP8E_BSE_LIB=vp8e_plugin/lib/libmfx_vp8e_bse_64.a
endif
    libmfx_vp8e_plugin_64_la_DEPENDENCIES += \
      $(top_builddir)/_studio/ipp/libipp-glibc.la
    libmfx_vp8e_plugin_64_la_LIBADD = \
      $(libmfx_vp8e_plugin_64_la_DEPENDENCIES) \
      $(VP8E_BSE_LIB) \
      $(LIBVA_LIBS) \
      -lpthread
    libmfx_vp8e_plugin_64_la_CFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      $(LIBVA_CFLAGS) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_plugin_64_la_CXXFLAGS = \
      $(MFX_CFLAGS_INTERNAL) \
      $(LIBVA_CFLAGS) \
      -DMFX_VA \
      -DAS_VP8E_PLUGIN
    libmfx_vp8e_plugin_64_la_LDFLAGS = \
      -Wl,--version-script=$(srcdir)/libmfx_vp8e_plugin.map \
      $(MFX_LDFLAGS) \
      $(MEDIASDK_LT_LDFLAGS)
endif
