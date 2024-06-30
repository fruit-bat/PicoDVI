package org.fruitbat.midipacker;

import java.util.HashMap;
import java.util.Map;

public class VoiceGroups {
    private Map<VoiceGroupKey, VoiceGroup> _groups = new HashMap<>();

    public VoiceGroup get(final VoiceGroupKey key) {
        final VoiceGroup g1 = _groups.get(key);
        if (g1 != null) return g1;
        final VoiceGroup g2 = new VoiceGroup(key);
        _groups.put(key, g2);
        return g2;
    }

    public VoiceGroup get(final int track, final int channel) {
        return get(new VoiceGroupKey(track, channel));
    }
}
