package org.fruitbat.midipacker;


public class PrSyn implements Syn {
	private final char[] _keys = new char[128];

	public PrSyn() {
		for(int i = 0; i < _keys.length; ++i) _keys[i] = '-';
	}
	@Override
	public void noteOn(long tick, final int track, final int channel, final int key, final int velocity) {
		_keys[key] = velocity == 0 ? '-' : (char)('0' + track);
	}
	@Override
	public void noteOff(long tick, final int track, final int channel, final int key, final int velocity) {
		_keys[key] = '-';
	}
	@Override
	public void bend(long tick, final int track, final int channel, final int amount) {
	}
	@Override
	public String toString() {
		return String.valueOf(_keys);
	}
}
