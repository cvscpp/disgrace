bin_PROGRAMS = disgrace

disgrace_SOURCES = \
    main.cpp \
    core/engine.cpp \
    core/engine.h \
    core/transport.cpp \
    core/transport.h \
    audio/jack_backend.cpp \
    audio/jack_backend.h \
    audio/mixer.cpp \
    audio/mixer.h \
    audio/dsp/gain.cpp \
    audio/dsp/gain.h \
    sequencer/pattern.cpp \
    sequencer/pattern.h \
    sequencer/track.cpp \
    sequencer/track.h \
    instruments/instrument.h \
    instruments/sample_instrument.cpp \
    instruments/sample_instrument.h \
    gui/main_window.cpp \
    gui/main_window.h \
    gui/tracker_widget.cpp \
    gui/tracker_widget.h \
    fileio/project.cpp \
    fileio/project.h \
    util/logger.cpp \
    util/logger.h

disgrace_CXXFLAGS = \
    $(JACK_CFLAGS) \
    $(FLTK_CFLAGS) \
    $(SNDFILE_CFLAGS) \
    $(SAMPLERATE_CFLAGS) \
    $(SNDTOUCH_CFLAGS)

disgrace_LDADD = \
    $(JACK_LIBS) \
    $(FLTK_LIBS) \
    $(SNDFILE_LIBS) \
    $(SAMPLERATE_LIBS) \
    $(SNDTOUCH_LIBS)
