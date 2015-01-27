AUTOMAKE_OPTIONS=foreign

INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/_studio/ipp/include \
  -I$(top_srcdir)/_studio/shared/include \
  -I$(top_srcdir)/_studio/shared/umc/core/umc/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm/include \
  -I$(top_srcdir)/_studio/shared/umc/core/vm_plus/include

umc_sources = \
  umc/src/umc_base_codec.cpp \
  umc/src/umc_default_frame_allocator.cpp \
  umc/src/umc_default_memory_allocator.cpp \
  umc/src/umc_frame_data.cpp \
  umc/src/umc_index.cpp \
  umc/src/umc_media_buffer.cpp \
  umc/src/umc_media_data.cpp \
  umc/src/umc_media_data_ex.cpp \
  umc/src/umc_par_reader.cpp \
  umc/src/umc_utils.cpp \
  umc/src/umc_va_base.cpp \
  umc/src/umc_video_data.cpp \
  umc/src/umc_video_decoder.cpp \
  umc/src/umc_video_encoder.cpp

vm_sources = \
  vm/src/vm_event_linux32.c \
  vm/src/vm_file.c \
  vm/src/vm_file_unix.c \
  vm/src/vm_interlocked_unix.c \
  vm/src/vm_mmap_linux32.c \
  vm/src/vm_mutex_linux32.c \
  vm/src/vm_semaphore_linux32.c \
  vm/src/vm_shared_object_linux32.c \
  vm/src/vm_strings_unix.c \
  vm/src/vm_sys_info_linux32.c \
  vm/src/vm_thread_linux32.c \
  vm/src/vm_time_linux32.c

vm_plus_sources = \
  vm_plus/src/mfx_vm++_pthread.cpp \
  vm_plus/src/mfx_vm++_pthread_unix.cpp \
  vm_plus/src/umc_event.cpp \
  vm_plus/src/umc_malloc.cpp \
  vm_plus/src/umc_mmap.cpp \
  vm_plus/src/umc_pendulum.cpp \
  vm_plus/src/umc_semaphore.cpp \
  vm_plus/src/umc_sys_info.cpp \
  vm_plus/src/umc_thread.cpp

noinst_LTLIBRARIES = libumc_core_merged.la

libumc_core_merged_la_SOURCES = \
  $(umc_sources) \
  $(vm_sources) \
  $(vm_plus_sources)
libumc_core_merged_la_LIBADD = @LIBOBJS@
libumc_core_merged_la_CFLAGS = $(MFX_CFLAGS_INTERNAL)
libumc_core_merged_la_CXXFLAGS = $(libumc_core_merged_la_CFLAGS)
