package org.fruitbat.midipacker;

import java.util.Iterator;
import java.util.LinkedList;


public class MidiPacker implements Syn {

    private final LinkedList<Voice> _voicesOn = new LinkedList<>();
    private final LinkedList<Voice> _voicesOff = new LinkedList<>();
    private final VoiceGroups _groups = new VoiceGroups();

    private final int _voiceCount;
    private final SynWriter _writer;

    private int _maxVoiceOnCount = 0;
	private final char[] _keys = new char[128];

    public MidiPacker(SynWriter writer, final int voiceCount) {
        _voiceCount = voiceCount;
        for(int i = 0; i < _keys.length; ++i) _keys[i] = '-';
        for(int i = 0 ; i < _voiceCount; ++i) {
            _voicesOff.addFirst( new Voice(i));
        }
        _writer = writer;
    }

    public int maxVoiceOnCount() {
        return _maxVoiceOnCount;
    }

    public void clearGroupModifiers(final Voice voice) {
        final VoiceGroup g = voice.group();
        if (g == null) return;
        if (g.bend() != null) {
            _writer.writeBend(voice.index(), 0);
        }
    }

    public void writeGroupModifiers(final Voice voice) {
        final VoiceGroup g = voice.group();
        if (g == null) return;
        if (g.bend() != null) {
            _writer.writeBend(voice.index(), g.bend());
        }                
    }

    public void soundOn(long tick, int track, int channel, int key, int velocity) {
        if (_voicesOff.size() < 1) throw new RuntimeException("Too few voices");
        final VoiceGroup group = _groups.get(track, channel);
        final Voice voice = _voicesOff.removeLast();
        // Update modifiers from the voice
        if (!group.equals(voice.group())) {
            clearGroupModifiers(voice);
            voice.assign(group);
            writeGroupModifiers(voice);
        }
        voice.on(key);
        _writer.writeVoiceOn(voice.index(), key, velocity);
        _keys[key] = (char)('0' + voice.index());
        _voicesOn.add(voice);
        final int voiceOnCount = _voicesOn.size();
        if (voiceOnCount > _maxVoiceOnCount) _maxVoiceOnCount = voiceOnCount;
    }

    public void soundOff(long tick, int track, int channel, int key, int velocity) {
        final VoiceGroupKey k = new VoiceGroupKey(track, channel);
        for (final Iterator<Voice> it =_voicesOn.iterator() ; it.hasNext(); ) {
            final Voice voice = it.next();
            if (voice.is(k, key)) {
                it.remove();
                _keys[key] = '-';
                _voicesOff.addFirst(voice);
                _writer.writeVoiceOff(voice.index(), velocity);
                break;
            }
        }
    } 

    @Override
    public void noteOn(long tick, int track, int channel, int key, int velocity) {
        if (velocity > 0) {
            soundOn(tick, track, channel, key, velocity);
        }
        else {
            soundOff(tick, track, channel, key, velocity);
        }

    }

    @Override
    public void noteOff(long tick, int track, int channel, int key, int velocity) {
        soundOff(tick, track, channel, key, velocity);
    }

	@Override
	public String toString() {
		return String.valueOf(_keys);
	}

	@Override
	public void bend(long tick, final int track, final int channel, final int amount) {
        final VoiceGroupKey groupKey = new VoiceGroupKey(track, channel);
        final VoiceGroup group = _groups.get(groupKey);
        group.bend(amount == 0 ? null : amount);
        for (final Iterator<Voice> it =_voicesOn.iterator() ; it.hasNext(); ) {
            final Voice voice = it.next();
            if (group.equals(voice.group())) {
                _writer.writeBend(voice.index(), amount);
            }
        }        
	}
}
