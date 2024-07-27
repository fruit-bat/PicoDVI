package org.fruitbat.midipacker;

/**
 * Something the synth can make a noise with
 */
public class Voice {
    private boolean _on = false;
    private VoiceGroup _group;
    private int _index;
    private int _key;

    public Voice(final int index) {
        _index = index;
        _group = null;
    }

    public VoiceGroup group() {
        return _group;
    }

    public boolean is(final int track, final int channel, final int key) {
        return _group != null && _group.key().is(track, channel) && _key == key;
    }
    public boolean is(final VoiceGroupKey groupKey, final int key) {
        return _group != null && _group.key().equals(groupKey) && _key == key;
    }

    public void assign(VoiceGroup group) {
        _group = group;
    }

    public void on(final int key) {
        _on = true;
        _key = key;
    }

    public void off() {
        _on = false;
    }

    public boolean on() {
        return _on;
    }

    public int index() {
        return _index;
    }
}
