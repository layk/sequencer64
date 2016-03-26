#ifndef SEQ64_SEQUENCE_HPP
#define SEQ64_SEQUENCE_HPP

/*
 *  This file is part of seq24/sequencer64.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          sequence.hpp
 *
 *  This module declares/defines the base class for handling
 *  patterns/sequences.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2016-03-26
 * \license       GNU GPLv2 or above
 *
 *  The functions add_list_var() and add_long_list() have been replaced by
 *  functions in the new midi_container module.
 *
 *  We've offloaded most of the trigger code to the triggers class in its own
 *  module, and now just call its member functions to do the actual work.
 */

#include <string>
#include <list>
#include <stack>

#include "easy_macros.h"
#include "event_list.hpp"

#ifdef PLATFORM_WINDOWS
#include "midibus_portmidi.hpp"
#else
#include "midibus.hpp"
#endif

#include "midi_container.hpp"           /* seq64::midi_container    */
#include "mutex.hpp"
#include "scales.h"                     /* key and scale constants  */
#include "triggers.hpp"                 /* seq64::triggers, etc.    */

/**
 *  Provides a new option to save the Time Signature and Tempo data that may
 *  be present in a MIDI file (in the first track) in the sequence object, and
 *  write them back to the MIDI file when saved again, in Sequencer64 format.
 *  The SeqSpec events that Seq24 and Sequencer64 save for these "events" are
 *  not readable by other MIDI applications, such as QTractor.  By enabling
 *  this macro, other sequencers can read the correct time-signature and tempo
 *  values.
 */

#define SEQ64_HANDLE_TIMESIG_AND_TEMPO

namespace seq64
{

class mastermidibus;

#ifdef SEQ64_PAUSE_SUPPORT
class perform;
#endif

/**
 *  Provides a set of methods for drawing certain items.
 *
 * \var DRAW_FIN
 *
 * \var DRAW_NORMAL_LINKED
 *
 * \var DRAW_NOTE_ON
 *
 * \var DRAW_NOTE_OFF
 */

enum draw_type
{
    DRAW_FIN = 0,
    DRAW_NORMAL_LINKED,
    DRAW_NOTE_ON,
    DRAW_NOTE_OFF
};

/**
 *  The sequence class is firstly a receptable for a single track of MIDI
 *  data read from a MIDI file or edited into a pattern.  More members than
 *  you can shake a stick at.
 */

class sequence
{

#ifdef SEQ64_PAUSE_SUPPORT
    friend class perform;               /* access to set_parent()   */
#endif

    friend class triggers;              /* will unfriend later      */

public:

    /**
     *  This enumeration is used in selecting events and note.  Se the
     *  select_note_events() and select_events() functions.
     *
     * \var e_select
     *      To select ...
     *
     * \var e_select_one
     *      To select ...
     *
     * \var e_is_selected
     *      The events are selected ...
     *
     * \var e_would_select
     *      The events would be selected ...
     *
     * \var e_deselect
     *      To deselect the event under the cursor.
     *
     * \var e_toggle_selection
     *      To toggle the selection of the event under the cursor.
     *
     * \var e_remove_one
     *      To remove one note under the cursor.
     */

    enum select_action_e
    {
        e_select,
        e_select_one,
        e_is_selected,
        e_would_select,
        e_deselect,
        e_toggle_selection,
        e_remove_one
    };

private:

    typedef std::stack<event_list> EventStack;

private:

    static event_list m_events_clipboard;

#ifdef SEQ64_PAUSE_SUPPORT

    /**
     *  For pause support, we need a way for the sequence to find out if JACK
     *  transport is active.  We can use the rc_settings flag(s), but JACK
     *  could be disconnected.  We could use a reference here, but, to avoid
     *  modifying the midifile class as well, we use a pointer.  It is set in
     *  perform::add_sequence().
     */

    perform * m_parent;

#endif

    /**
     *  This list holds the current pattern/sequence events.
     */

    event_list m_events;
    triggers m_triggers;
    EventStack m_events_undo;
    EventStack m_events_redo;
    event_list::iterator m_iterator_play;
    event_list::iterator m_iterator_draw;

    /**
     *  Contains the proper MIDI channel for this sequence.  However, if this
     *  value is EVENT_NULL_CHANNEL (0xFF), then this sequence is an SMF 0
     *  track, and has no single channel.
     */

    midibyte m_midi_channel;

    /**
     *  Contains the proper MIDI bus number for this sequence.
     */

    midibyte m_bus;

    /**
     *  Provides a flag for the song playback mode muting.
     */

    bool m_song_mute;

    /**
     *  Provides a member to hold the polyphonic step-edit note counter.
     */

    int m_notes_on;

    /**
     *  Provides the master MIDI buss which handles the output of the sequence
     *  to the proper buss and MIDI channel.
     */

    mastermidibus * m_masterbus;

    /**
     *  Provides a "map" for Note On events.  It is used when muting, to shut
     *  off the notes that are playing.
     */

    int m_playing_notes[SEQ64_MIDI_NOTES_MAX];

    /**
     *  Indicates if the sequence was playing.
     */

    bool m_was_playing;

    /**
     *  True if sequence playback currently is in progress for this sequence.
     */

    bool m_playing;

    /**
     *  True if sequence recording currently is in progress for this sequence.
     */

    bool m_recording;

    bool m_quantized_rec;
    bool m_thru;
    bool m_queued;

    /**
     *  These flags indicate that the content of the sequence has changed due
     *  to recording, editing, performance management, or even (?) a
     *  name change.
     */

    bool m_dirty_main;
    bool m_dirty_edit;
    bool m_dirty_perf;
    bool m_dirty_names;

    /**
     *  Indicates that the sequence is currently being edited.
     */

    bool m_editing;

    bool m_raise;

    /**
     *  Provides the name/title for the sequence.
     */

    std::string m_name;

    /**
     *  These members manage where we are in the playing of this sequence,
     *  including triggering.
     */

    midipulse m_last_tick;
    midipulse m_queued_tick;
    midipulse m_trigger_offset;

    /**
     *  This constant provides ...?
     */

    const int m_maxbeats;

    /**
     *  Holds the PPQN value for this sequence, so that we don't have to rely
     *  on a global constant value.
     */

    int m_ppqn;

    /**
     *  A new member so that the sequence number is carried along with the
     *  sequence.  This number is set in the perform::install_sequence()
     *  function.
     */

    int m_seq_number;

    /**
     *  Holds the length of the sequence in pulses (ticks).  This value should
     *  be a power of two when used as a bar unit.
     */

    midipulse m_length;

    /**
     *  The size of snap in units of pulses (ticks).  It starts out as the
     *  value m_ppqn / 4.
     */

    midipulse m_snap_tick;

    /**
     *  Provides the number of beats per bar used in this sequence.  Defaults
     *  to 4.  Used by the sequence editor to mark things in correct time on
     *  the user-interface.
     */

    int m_time_beats_per_measure;

    /**
     *  Provides with width of a beat.  Defaults to 4, which means the beat is
     *  a quarter note.  A value of 8 would mean it is an eighth note.  Used
     *  by the sequence editor to mark things in correct time on the
     *  user-interface.
     */

    int m_time_beat_width;

#ifdef SEQ64_HANDLE_TIMESIG_AND_TEMPO

    /**
     *  Augments the beats/bar and beat-width with the additional values
     *  included in a Time Signature meta event.  This value provides the
     *  number of MIDI clocks between metronome clicks.  The default value of
     *  this item is 24.  It can also be read from some SMF 1 files, such as
     *  our hymne.mid example.
     */

    int m_clocks_per_metronome;

    /**
     *  Augments the beats/bar and beat-width with the additional values
     *  included in a Time Signature meta event.  This value provides the
     *  number of notated 32nd notes in a MIDI quarter note (24 MIDI clocks).
     *  The usual (and default) value of this parameter is 8; some sequencers
     *  allow this to be changed.
     */

    int m_32nds_per_quarter;

    /**
     *  Augments the beats/bar and beat-width with the additional values
     *  included in a Tempo meta event.  This value can be extracted from the
     *  beats-per-minute value (mastermidibus::m_beats_per_minute), but here
     *  we set it to 0 by default, indicating that we don't want to write it.
     *  Otherwise, it can be read from a MIDI file, and saved here to be
     *  restored later.
     */

    int m_us_per_quarter_note;

#endif  // SEQ64_HANDLE_TIMESIG_AND_TEMPO

    /**
     *  The volume to be used when recording.
     */

    int m_rec_vol;

    /**
     *  Holds a copy of the musical key for this sequence, which we now
     *  support writing to this sequence.  If the value is
     *  SEQ64_KEY_OF_C, then there is no musical key to be set.
     */

    midibyte m_musical_key;

    /**
     *  Holds a copy of the musical scale for this sequence, which we now
     *  support writing to this sequence.  If the value is the enumeration
     *  value c_scale_off, then there is no musical scale to be set.
     */

    midibyte m_musical_scale;

    /**
     *  Holds a copy of the background sequence number for this sequence,
     *  which we now support writing to this sequence.  If the value is
     *  greater than max_sequence(), then there is no background sequence to
     *  be set.
     */

    int m_background_sequence;

    /**
     *  Provides locking for the sequence.  Made mutable for use in
     *  certain locked getter functions.
     */

    mutable mutex m_mutex;

private:

    /*
     * We're going to replace this operator with the more specific
     * partial_assign() function.
     */

    sequence & operator = (const sequence & rhs);

public:

    sequence (int ppqn = SEQ64_USE_DEFAULT_PPQN);
    ~sequence ();

    void partial_assign (const sequence & rhs);

    /**
     * \getter m_events
     */

    event_list & events ()
    {
        return m_events;
    }

    /**
     * \getter m_events
     */

    const event_list & events () const
    {
        return m_events;
    }

    /**
     * \getter m_events.any_selected_notes()
     */

    bool any_selected_notes () const
    {
        return m_events.any_selected_notes();
    }

    /**
     * \getter m_triggers
     */

    triggers::List & triggerlist ()
    {
        return m_triggers.triggerlist();
    }

    /**
     * \getter m_seq_number
     */

    int number () const
    {
        return m_seq_number;
    }

    /**
     * \setter m_seq_number
     *      This setter will set the sequence number only if it has not
     *      already been set.
     */

    void number (int seqnum)
    {
        if (seqnum >= 0 && m_seq_number == (-1))
            m_seq_number = seqnum;
    }

    int event_count () const;
    void push_undo ();
    void pop_undo ();
    void pop_redo ();
    void push_trigger_undo ();
    void pop_trigger_undo ();
    void set_name (const std::string & name);
    void set_name (char * name);
    void set_measures (int lengthmeasures);
    int get_measures ();

    /**
     * \getter m_ppqn
     *      Provided as a convenience for the editable_events class.
     */

    int get_ppqn () const
    {
        return m_ppqn;
    }

    void set_beats_per_bar (int beatspermeasure);

    /**
     * \getter m_time_beats_per_measure
     */

    int get_beats_per_bar () const
    {
        return m_time_beats_per_measure;
    }

    void set_beat_width (int beatwidth);

    /**
     * \getter m_time_beat_width
     *
     * \threadsafe
     */

    int get_beat_width () const
    {
        return m_time_beat_width;
    }

#ifdef SEQ64_HANDLE_TIMESIG_AND_TEMPO

    /**
     * \setter m_clocks_per_metronome
     */

    void clocks_per_metronome (int cpm)
    {
        m_clocks_per_metronome = cpm;       // needs validation
    }

    /**
     * \getter m_clocks_per_metronome
     */

    int clocks_per_metronome () const
    {
        return m_clocks_per_metronome;
    }

    /**
     * \setter m_32nds_per_quarter
     */

    void set_32nds_per_quarter (int tpq)
    {
        m_32nds_per_quarter = tpq;              // needs validation
    }

    /**
     * \getter m_32nds_per_quarter
     */

    int get_32nds_per_quarter () const
    {
        return m_32nds_per_quarter;
    }

    /**
     * \setter m_us_per_quarter_note
     */

    void us_per_quarter_note (int upqn)
    {
        m_us_per_quarter_note = upqn;       // needs validation
    }

    /**
     * \getter m_us_per_quarter_note
     */

    int us_per_quarter_note () const
    {
        return m_us_per_quarter_note;
    }

#endif  // SEQ64_HANDLE_TIMESIG_AND_TEMPO

    void set_rec_vol (int rec_vol);

    /**
     * \setter m_song_mute
     */

    void set_song_mute (bool mute)
    {
        m_song_mute = mute;
    }

    /**
     * \getter m_song_mute
     */

    bool get_song_mute () const
    {
        return m_song_mute;
    }

    /**
     * \getter m_name pointer
     * \deprecated
     */

    const char * get_name () const
    {
        return m_name.c_str();
    }

    /**
     * \getter m_name
     */

    const std::string & name () const
    {
        return m_name;
    }

    /**
     * \setter m_editing
     */

    void set_editing (bool edit)
    {
        m_editing = edit;
    }

    /**
     * \getter m_editing
     */

    bool get_editing () const
    {
        return m_editing;
    }

    /**
     * \setter m_raise
     */

    void set_raise (bool edit)
    {
        m_raise = edit;
    }

    /**
     * \getter m_raise
     */

    bool get_raise (void) const
    {
        return m_raise;
    }

    void set_length (midipulse len, bool adjust_triggers = true); /* in ticks */

    /**
     * \getter m_length
     */

    midipulse get_length () const
    {
        return m_length;
    }

    midipulse get_last_tick ();
    void set_last_tick (midipulse tick);

    /**
     *  Some MIDI file errors and other things can lead to an m_length of 0,
     *  which causes arithmetic errors when m_last_tick is modded against it.
     *  This function replaces the "m_last_tick % m_length", returning
     *  m_last_tick if m_length is 0 or 1.
     */

    midipulse mod_last_tick ()
    {
        return (m_length > 1) ?  (m_last_tick % m_length) : m_last_tick ;
    }

    void set_playing (bool);

    /**
     * \getter m_playing
     */

    bool get_playing () const
    {
        return m_playing;
    }

    /**
     *  Toggles the playing status of this sequence.
     */

    void toggle_playing ()
    {
        set_playing(! get_playing());
    }

    void toggle_queued ();
    void off_queued ();

    /**
     * \getter m_queued
     */

    bool get_queued () const
    {
        return m_queued;
    }

    /**
     * \getter m_queued_tick
     */

    midipulse get_queued_tick () const
    {
        return m_queued_tick;
    }

    /**
     *  Helper function for perform.
     */

    bool check_queued_tick (midipulse tick) const
    {
        return get_queued() && (get_queued_tick() <= tick);
    }

    void set_recording (bool);

    /**
     * \getter m_recording
     */

    bool get_recording () const
    {
        return m_recording;
    }

    void set_snap_tick (int st);
    void set_quantized_rec (bool qr);

    /**
     * \getter m_quantized_rec
     */

    bool get_quantized_rec () const
    {
        return m_quantized_rec;
    }

    void set_thru (bool);

    /**
     * \getter m_thru
     */

    bool get_thru () const
    {
        return m_thru;
    }

    bool is_dirty_main ();
    bool is_dirty_edit ();
    bool is_dirty_perf ();
    bool is_dirty_names ();
    void set_dirty_mp ();
    void set_dirty ();

    /**
     * \getter m_midi_channel
     */

    midibyte get_midi_channel () const
    {
        return m_midi_channel;
    }

    /**
     *  Returns true if this sequence is an SMF 0 sequence.
     */

    bool is_smf_0 () const
    {
        return m_midi_channel == EVENT_NULL_CHANNEL;
    }

    void set_midi_channel (midibyte ch);
    void print ();
    void print_triggers ();
    void play (midipulse tick, bool playback_mode);
    bool add_event (const event & er);
    void add_trigger
    (
        midipulse tick, midipulse len,
        midipulse offset = 0, bool adjust_offset = true
    );
    void split_trigger (midipulse tick);
    void grow_trigger (midipulse tick_from, midipulse tick_to, midipulse len);
    void del_trigger (midipulse tick);
    bool get_trigger_state (midipulse tick);
    bool select_trigger (midipulse tick);
    bool unselect_triggers ();
    bool intersect_triggers
    (
        midipulse position, midipulse & start, midipulse & ender
    );
    bool intersect_notes
    (
        midipulse position, midipulse position_note,
        midipulse & start, midipulse & ender, int & note
    );
    bool intersect_events
    (
        midipulse posstart, midipulse posend,
        midibyte status, midipulse & start
    );
    void del_selected_trigger ();
    void cut_selected_trigger ();
    void copy_selected_trigger ();
    void paste_trigger ();
    bool move_selected_triggers_to
    (
        midipulse tick, bool adjust_offset, int which = 2
    );
    midipulse selected_trigger_start ();
    midipulse selected_trigger_end ();
    midipulse get_max_trigger ();
    void move_triggers (midipulse start_tick, midipulse distance, bool direction);
    void copy_triggers (midipulse start_tick, midipulse distance);
    void clear_triggers ();

    /**
     * \getter m_trigger_offset
     */

    midipulse get_trigger_offset () const
    {
        return m_trigger_offset;
    }

    void set_midi_bus (char mb);

    /**
     * \getter m_bus
     */

    char get_midi_bus () const
    {
        return m_bus;
    }

    void set_master_midi_bus (mastermidibus * mmb);
    int select_note_events
    (
        midipulse tick_s, int note_h,
        midipulse tick_f, int note_l, select_action_e action
    );
    int select_events
    (
        midipulse tick_s, midipulse tick_f,
        midibyte status, midibyte cc, select_action_e action
    );
    int select_events
    (
        midibyte status, midibyte cc, bool inverse = false
    );
    int get_num_selected_notes () const;
    int get_num_selected_events (midibyte status, midibyte cc) const;
    void select_all ();
    void copy_selected ();
    void cut_selected (bool copyevents = true);
    void paste_selected (midipulse tick, int note);
    void get_selected_box
    (
        midipulse & tick_s, int & note_h, midipulse & tick_f, int & note_l
    );
    void get_clipboard_box
    (
        midipulse & tick_s, int & note_h, midipulse & tick_f, int & note_l
    );
    void move_selected_notes (midipulse deltatick, int deltanote);
    void add_note
    (
        midipulse tick, midipulse len, int note, bool paint = false
    );
    void add_event
    (
        midipulse tick, midibyte status,
        midibyte d0, midibyte d1, bool paint = false
    );
    void stream_event (event & ev);
    bool change_event_data_range
    (
        midipulse tick_s, midipulse tick_f,
        midibyte status, midibyte cc,
        int d_s, int d_f
    );
    void increment_selected (midibyte status, midibyte control);
    void decrement_selected (midibyte status, midibyte control);
    void grow_selected (midipulse deltatick);
    void stretch_selected (midipulse deltatick);
    void remove_marked ();
    void mark_selected ();
    void unpaint_all ();
    void unselect ();
    void verify_and_link ();
    void link_new ();

    /**
     *  Resets everything to zero.  This function is used when the sequencer
     *  stops.  This function currently sets m_last_tick = 0, but we would
     *  like to avoid that if doing a pause, rather than a stop, of playback.
     *  However, commenting out this setting doesn't have any effect that we
     *  can see with a quick look at the user-interface.
     *
     * \param tick
     *      Provides the optional tick value to set as "0".  It defaults to 0.
     */

    void zero_markers ()
    {
        set_last_tick(0);
    }

    void play_note_on (int note);
    void play_note_off (int note);
    void off_playing_notes ();
    void reset (bool live_mode, bool pause = false);
    void reset_draw_marker ();
    void reset_draw_trigger_marker ();
    draw_type get_next_note_event
    (
        midipulse * tick_s, midipulse * tick_f, int * note,
        bool * selected, int * velocity
    );
    bool get_minmax_note_events (int & lowest, int & highest);
    bool get_next_event
    (
        midibyte status, midibyte cc,
        midipulse * tick, midibyte * d0, midibyte * d1,
        bool * selected
    );
    bool get_next_event (midibyte * status, midibyte * cc);
    bool get_next_trigger
    (
        midipulse * tick_on, midipulse * tick_off,
        bool * selected, midipulse * tick_offset
    );
    void fill_container (midi_container & c, int tracknumber);
    void quantize_events
    (
        midibyte status, midibyte cc,
        midipulse snap_tick, int divide, bool linked = false
    );
    void transpose_notes (int steps, int scale);

    /**
     * \getter m_musical_key
     */

    midibyte musical_key () const
    {
        return m_musical_key;
    }

    /**
     * \setter m_musical_key
     */

    void musical_key (int key)
    {
        if (key >= SEQ64_KEY_OF_C && key < SEQ64_OCTAVE_SIZE)
            m_musical_key = midibyte(key);
    }

    /**
     * \getter m_musical_scale
     */

    midibyte musical_scale () const
    {
        return m_musical_scale;
    }

    /**
     * \setter m_musical_scale
     */

    void musical_scale (int scale)
    {
        if (scale >= int(c_scale_off) && scale < int(c_scale_size))
            m_musical_scale = midibyte(scale);
    }

    /**
     * \getter m_background_sequence
     */

    int background_sequence () const
    {
        return m_background_sequence;
    }

    /**
     * \setter m_background_sequence
     *      Only partial validation at present, we do not want the upper
     *      limit to be hard-wired at this time.  Disabling the sequence
     *      number (setting it to SEQ64_SEQUENCE_LIMIT) is valid.
     */

    void background_sequence (int bs)
    {
        if (SEQ64_IS_LEGAL_SEQUENCE(bs))
            m_background_sequence = bs;
    }

    void show_events () const;
    void copy_events (const event_list & newevents);

private:

    void set_parent (perform * p);
    void put_event_on_bus (event & ev);
    void set_trigger_offset (midipulse trigger_offset);
    void split_trigger (trigger & trig, midipulse splittick);
    void adjust_trigger_offsets_to_length (midipulse newlen);
    midipulse adjust_offset (midipulse offset);
    void remove (event_list::iterator i);
    void remove (event & e);
    void remove_all ();

};          // class sequence

}           // namespace seq64

#endif      // SEQ64_SEQUENCE_HPP

/*
 * sequence.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

