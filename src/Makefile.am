bin_PROGRAMS = disgrace

disgrace_SOURCES = \
    main.cpp \
    core/command.h \
    core/engine.cpp \
    core/engine.h \
    core/engine_command.h \
    core/engine_save.cpp \
    core/MasterBus.cpp \
    core/MasterBus.h \
    core/metronome.cpp \
    core/metronome.h \
    core/transport.cpp \
    core/transport.h \
    core/undo_stack.cpp \
    core/undo_stack.h \
    audio/adsr.h \
    audio/audio_backend.h \
    audio/jack_backend.cpp \
    audio/jack_backend.h \
    audio/resampler.cpp \
    audio/resampler.h \
    audio/sample_buffer.cpp \
    audio/sample_buffer.h \
    audio/sample_data.h \
    audio/sample_voice.cpp \
    audio/sample_voice.h \
    audio/timestretch.cpp \
    audio/timestretch.h \
    audio/voice.h \
    dsp/delay.h \
    dsp/dsp_chain.cpp \
    dsp/dsp_chain.h \
    dsp/dsp.h \
    dsp/gain.h \
    edit/cmd_clear_block.h \
    edit/cmd_paste_block.h \
    edit/cmd_set_note.h \
    edit/edit_command.h \
    edit/undo_stack.h \
    gui/main_window.cpp \
    gui/main_window.h \
    gui/model/mixer_model.cpp \
    gui/model/mixer_model.h \
    gui/tracker_view.cpp \
    gui/tracker_view.h \
    gui/transport_panel.cpp \
    gui/transport_panel.h \
    gui/transportbar.cpp \
    gui/transportbar.h \
    instrument/instrument.cpp \
    instrument/instrument.h \
    instrument/instrument_rack.cpp \
    instrument/instrument_rack.h \
    instrument/sample_instrument.cpp \
    instrument/sample_instrument.h \
    io/audio_file.cpp \
    io/audio_file.h \
    io/project_archive.cpp \
    io/project_archive.h \
    io/song_serializer.cpp \
    io/song_serializer.h \
    midi/MidiInput.cpp \
    midi/MidiInput.h \
    midi/MidiQueue.h \
    mixer/track.cpp \
    mixer/track.h \
    sequencer/automation.cpp \
    sequencer/automation.h \
    sequencer/note.h \
    sequencer/pattern.h \
    sequencer/sequencer.cpp \
    sequencer/sequencer.h \
    sequencer/timing.cpp \
    sequencer/timing.h \
    util/ringbuffer.h \
    analysis/fft_analyzer.cpp \
    analysis/fft_analyzer.h

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
