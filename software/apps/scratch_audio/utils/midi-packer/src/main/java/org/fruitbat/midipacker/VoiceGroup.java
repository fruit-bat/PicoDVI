package org.fruitbat.midipacker;

public class VoiceGroup {
    private final VoiceGroupKey _key;
    private final int _id;

    public VoiceGroup(final VoiceGroupKey key, final int id) {
        _key = key;
        _id = id;
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

    public int id() {
        return _id;
    }
}
