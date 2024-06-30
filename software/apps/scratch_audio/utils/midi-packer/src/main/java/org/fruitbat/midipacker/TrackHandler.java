package org.fruitbat.midipacker;


import javax.sound.midi.MetaMessage;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.Track;


public class TrackHandler { 
	private final Track _track;
	private final int _size;
	private final int _index;
	private int _pos;
	private final Syn _syn;
	private final TempoHandler _tempoHandler;

	public TrackHandler(
			final Track track,
			final int index,
			final Syn syn,
			final TempoHandler metaHandler) {
		_track = track;
		_index = index;
		_size = track.size();
		_pos = 0;
		_syn = syn;
		_tempoHandler = metaHandler;
	}

	public boolean active() {
		return _pos < _size;
	}

	public void tick(final long tick) {
		while(_pos < _size) {
			final MidiEvent event = _track.get(_pos);
			if (event.getTick() > tick) return;
			_pos++;

			//		System.out.print("@" + event.getTick() + " Track " + _index + " ");
			final MidiMessage message = event.getMessage();

			if (message instanceof ShortMessage) {
				final ShortMessage sm = (ShortMessage) message;
				final int channel = sm.getChannel();
				//              System.out.print("Channel " + channel + " ");
				switch(sm.getCommand()) {
				case ShortMessage.NOTE_ON: {
					noteOn(tick, channel, sm.getData1(), sm.getData2());
					break;
				}
				case ShortMessage.NOTE_OFF: {
					noteOff(tick, channel, sm.getData1(), sm.getData2());
					break;
				}
				case ShortMessage.PITCH_BEND: {
					final int b = ((sm.getData1() & 0x7f) | ((sm.getData2() & 0x7f) << 7)) - 8192;
					_syn.bend(tick, _index, channel, b);
					System.out.println("PITCH_BEND: " + sm.getData1() + " " + sm.getData2() + " " + b);
					break;
				}				
				default: {
					System.out.println("Command: " + sm.getCommand());
					break;
				}
				}
			}
			else if (message instanceof MetaMessage) {
				final MetaMessage mm = (MetaMessage) message;
				if (mm.getType() == 0x51) {
					// Tempo message
					// See https://www.recordingblogs.com/wiki/midi-set-tempo-meta-message
					final byte[] d = mm.getData();
					final int u = ((d[0] & 0xff) << 16) + ((d[1] & 0xff) << 8) + (d[2] & 0xff);
					//System.out.println("Tempo: " + u);
					_tempoHandler.setTempo(u);
				}
			}
		}
	}

	public void noteOn(long tick, final int channel, final int key, final int velocity) {
		_syn.noteOn(tick, _index, channel, key, velocity);
	}

	public void noteOff(long tick, final int channel, final int key, final int velocity) {
		_syn.noteOff(tick, _index, channel, key, velocity);
	}

}
