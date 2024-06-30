package org.fruitbat.midipacker;

public class SynWriterNull implements SynWriter {

    @Override
    public void writeTempo(long uspqb) {
    }

    @Override
    public void writeVoiceOn(int index, int key, int velocity) {
    }

    @Override
    public void writeVoiceOff(int index, int velocity) {
    }

    @Override
    public void writeTime(long absTicks) {
    }

    @Override
    public void writePPQ(int ppq) {
    }

    @Override
    public void writeEnd() {
    }

    @Override
    public void writeStart() {
    }

    @Override
    public void writeBend(final int index, final int amount) {
    }
}
