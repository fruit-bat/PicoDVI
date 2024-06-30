package org.fruitbat.midipacker;

public interface SynWriter {
    public void writeStart();
    public void writePPQ(final int ppq);
    public void writeTempo(final long uspqb);
    public void writeVoiceOn(final int index, final int key, final int velocity);
    public void writeVoiceOff(final int index, final int velocity);
    public void writeTime(final long absTicks);
    public void writeBend(final int index, final int amount);
    public void writeEnd();
}
