package org.fruitbat.midipacker;


public interface Syn {
	public void noteOn(long tick, final int track, final int channel, final int key, final int velocity);
	public void noteOff(long tick, final int track, final int channel, final int key, final int velocity);
}
