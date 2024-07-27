package org.fruitbat.midipacker;

public class CompSyn implements Syn {
	private final Syn[] _syns;

	public CompSyn(final Syn[] syns) {
		_syns = syns;
	}
	@Override
	public void noteOn(long tick, final int track, final int channel, final int key, final int velocity) {
		for (final Syn _syn : _syns) _syn.noteOn(tick, track, channel, key, velocity);
	}
	@Override
	public void noteOff(long tick, final int track, final int channel, final int key, final int velocity) {
		for (final Syn _syn : _syns) _syn.noteOff(tick, track, channel, key, velocity);
	}
	@Override
	public void bend(long tick, final int track, final int channel, final int amount) {
		for (final Syn _syn : _syns) _syn.bend(tick, track, channel, amount);
	}
}
