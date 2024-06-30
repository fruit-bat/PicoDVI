package org.fruitbat.midipacker;

import java.io.PrintStream;

enum SynCommands {
    SynCmdPPQ,
    SynCmdTempo,
    SynCmdOn,
    SynCmdOff,
    SynCmdBend,
    SynCmdTime,
    SynCmdEnd
}

public class SynWriter1 implements SynWriter {

    private long _currTick;
    private long _prevTick;
    private PrintStream _writer;

    private void println(final String s) {
        try{
            _writer.println(s);
        }
        catch(final Exception e) {
            throw new RuntimeException(e);
        }
    }

    private byte[] split(final long l, final int n) {
        final byte[] b = new byte[n];
        for(int i = 0 ; i < n; ++ i) {
            b[i] = (byte)((l >> (i << 3)) & 0xff);
        }
        return b;
    }

    private String nbytes(final long l, final int n) {
        StringBuffer sb = new StringBuffer();
        final byte[] b = split(l, n);
        for(int i = n - 1; i >= 0; --i) {
            sb.append(String.format("0x%02X, ", b[i]));
        }
        return sb.toString();
    }

    public SynWriter1(PrintStream writer) {
        _currTick = 0;
        _prevTick = 0;
        _writer = writer;
    }

    public void writeStart() {
        println("unsigned char syn_notes[] = {");
    }

    @Override
    public void writeTempo(long uspqb) {
        writeTimeDelta();
        println(String.format("  0x%02X, %s // Tempo %d us per quater beat", 
            SynCommands.SynCmdTempo.ordinal(),
            nbytes(uspqb, 3),
            uspqb));
    }

    @Override
    public void writeVoiceOn(int index, int key, int velocity) {
        writeTimeDelta();
        println(String.format("  0x%02X, 0x%02X, 0x%02X, 0x%02X, // Voice on %d, key %d, velocity %d",
            SynCommands.SynCmdOn.ordinal(),
            index,
            key,
            velocity,
            index,
            key,
            velocity));        
    }

    @Override
    public void writeVoiceOff(int index, int velocity) {
        writeTimeDelta();
        println(String.format("  0x%02X, 0x%02X, 0x%02X, // Voice off %d, velocity %d",
            SynCommands.SynCmdOff.ordinal(),
            index,
            velocity,
            index,
            velocity));        
    }

    @Override
    public void writeTime(long tick) {
        _currTick = tick;
    }

    @Override
    public void writePPQ(int ppq) {
        println(String.format("  0x%02X, %s // PPQ %d",
            SynCommands.SynCmdPPQ.ordinal(),
            nbytes(ppq, 2),
            ppq));
    }

    @Override
    public void writeEnd() {
        writeTimeDelta();
        println(String.format("  0x%02X // End", SynCommands.SynCmdEnd.ordinal()));
        println("};\n");
    }

    private void writeTimeDelta() {
        if (_currTick != _prevTick) {
            long delta = _currTick - _prevTick;
            _prevTick = _currTick;
            while(delta > 0) {
                long d = delta >= (1 << 16) ? (1 << 16) - 1 : delta;
                delta -= d;
                println(String.format("  0x%02X, %s // Time delta %d ticks", 
                    SynCommands.SynCmdTime.ordinal(),
                    nbytes(d, 2), 
                    d));
            }
        }
    }

    @Override
    public void writeBend(final int index, final int amount) {
        writeTimeDelta();
        println(String.format("  0x%02X, 0x%02X, %s // Bend voice %d, amount %d",
            SynCommands.SynCmdBend.ordinal(),
            index,
            nbytes(amount, 2),
            index,
            amount));  
    }
}
