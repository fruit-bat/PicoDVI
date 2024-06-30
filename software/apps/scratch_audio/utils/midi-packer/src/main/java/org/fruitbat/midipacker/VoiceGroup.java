package org.fruitbat.midipacker;

public class VoiceGroup {
    private VoiceGroupKey _key;

    private Integer _bend = null;

    public VoiceGroup(final VoiceGroupKey key) {
        _key = key;
    }

    public VoiceGroupKey key() {
        return _key;
    }

    @Override
    public boolean equals(final Object o) {
        if (o == null || !(o instanceof VoiceGroup)) return false;
        final VoiceGroup g = (VoiceGroup)o;
        return _key.equals(g._key);
    }    

    public void bend(Integer bend) {
        _bend = bend;
    }

    public Integer bend() {
        return _bend;
    }
}
