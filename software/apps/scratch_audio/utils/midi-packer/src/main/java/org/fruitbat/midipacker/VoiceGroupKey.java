package org.fruitbat.midipacker;

public class VoiceGroupKey {
    private int _track;
    private int _channel;

    public VoiceGroupKey(final int track, final int channel) {
        _track = track;
        _channel = channel;
    }

    @Override
    public int hashCode() {
        return (_track << 8) + _channel;
    }

    @Override
    public boolean equals(final Object o) {
        if (o == null || !(o instanceof VoiceGroupKey)) return false;
        final VoiceGroupKey k = (VoiceGroupKey)o;
        return _track == k._track && _channel == k._channel;
    }

    public boolean is(final int track, final int channel) {
        return _track == track && _channel == channel; 
    }
}
