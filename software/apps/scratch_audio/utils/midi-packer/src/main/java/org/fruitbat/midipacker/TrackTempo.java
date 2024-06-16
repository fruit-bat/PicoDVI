package org.fruitbat.midipacker;


import javax.sound.midi.Sequence;

class TrackTempo implements TempoHandler {
	final double division;
	final int ppq;
	final long trackLengthTicks;
	final double bpm = 60.0f;
	double tickSeconds;
	final double stepFrequency;
	final double stepPeriod;
	double ticksPerStep;
	long stepsPerTick;

	public TrackTempo(final Sequence sequence) {
		division = sequence.getDivisionType();
		ppq = sequence.getResolution();
		trackLengthTicks = sequence.getTickLength(); // Is this ppq?
		stepFrequency = 44100f * 2f;
		stepPeriod = 1d / stepFrequency;
		tickSeconds = 48 / (bpm * ppq);
		ticksPerStep = 1d / (tickSeconds * stepFrequency);
		stepsPerTick = (long)(tickSeconds * stepFrequency);
	}

	@Override
	public void setTempo(final int u) {
		// u is microseconds per quarter note
		// https://mido.readthedocs.io/en/latest/midi_files.html#:~:text=Unlike%20music%2C%20tempo%20in%20MIDI,is%20120%20beats%20per%20minute.
		tickSeconds = u / (1000000d * ppq);
		ticksPerStep = 1d / (tickSeconds * stepFrequency);
		stepsPerTick = (long)(tickSeconds * stepFrequency);
	}

	public long stepsPerTick() {
		return stepsPerTick;
	}

	final public double stepPeriod() {
		return stepPeriod;
	}
}
