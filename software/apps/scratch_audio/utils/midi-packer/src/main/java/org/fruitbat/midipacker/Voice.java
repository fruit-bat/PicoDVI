package org.fruitbat.midipacker;

/**
 * Something the synth can make a noise with
 */
public class Voice {
    private boolean _on = false;
    private int _index;
    private int _track;
    private int _channel;
    private int _key;

    public Voice(final int index) {
        _index = index;
    }

    public boolean is(final int track, final int channel, final int key) {
        return _track == track && _channel == channel && _key == key;
    }

    public void on(final int track, final int channel, final int key) {
        _on = true;
        _track = track;
        _channel = channel;
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
