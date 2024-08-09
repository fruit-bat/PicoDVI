#pragma once
#include "pico/stdlib.h"

#define US_UINT8_DLIST_NULL 0xff

typedef struct {
    uint8_t last;
    uint8_t next;
    uint8_t self; // TODO remove ??
} UsUint8DlistEntry;

typedef struct {
    uint8_t head;
    uint8_t tail;
} UsUint8DlistAnchor;

inline void us_uint8_dlist_anchor_init(UsUint8DlistAnchor *anchor) {
    anchor->head = US_UINT8_DLIST_NULL;
    anchor->tail = US_UINT8_DLIST_NULL;
}

inline void us_uint8_dlist_entry_init(UsUint8DlistEntry *entry, uint8_t self) {
    entry->last = US_UINT8_DLIST_NULL;
    entry->next = US_UINT8_DLIST_NULL;
    entry->self = self;
}

inline void us_uint8_dlist_anchor_init_array(UsUint8DlistAnchor *anchor, uint32_t count) {
    for(uint32_t i = 0; i < count; ++i) {
       us_uint8_dlist_anchor_init(&anchor[i]);
    }
}

inline void us_uint8_dlist_entry_init_array(UsUint8DlistEntry *entry, uint32_t count) {
    for(uint32_t i = 0; i < count; ++i) {
       us_uint8_dlist_entry_init(&entry[i], i);
    }
}

inline void us_uint8_dlist_unlink(
    UsUint8DlistAnchor *anchor,
    void *entries,
    UsUint8DlistEntry *(*get_entry)(void *entries, uint8_t index),
    uint8_t index
) {
    UsUint8DlistEntry *entry = get_entry(entries, index);

    if (entry->next == US_UINT8_DLIST_NULL) {
        // entry at the end of the list
        if (entry->self == anchor->tail) anchor->tail = entry->last;
    }
    else {
        // entry not at the end of the list
        get_entry(entries, entry->next)->last = entry->last;
    }

    if (entry->last == US_UINT8_DLIST_NULL) {
        // entry at the start of the list
        if (entry->self == anchor->head) anchor->head = entry->next;
    }
    else {
        // entry not at the start of the list
        get_entry(entries, entry->last)->next = entry->next;
    }

    // Not strictly necessary, but tidy
    entry->next = US_UINT8_DLIST_NULL;
    entry->last = US_UINT8_DLIST_NULL;
}

inline void us_uint8_dlist_link_head(
    UsUint8DlistAnchor *anchor,
    void *entries,
    UsUint8DlistEntry *(*get_entry)(void *entries, uint8_t index),
    uint8_t index
) {
    UsUint8DlistEntry *entry = get_entry(entries, index);
    entry->next = anchor->head;
    if (anchor->head != US_UINT8_DLIST_NULL) {
        get_entry(entries, anchor->head)->last = index;
    }
    entry->last = US_UINT8_DLIST_NULL;
    anchor->head = index;
    if (anchor->tail == US_UINT8_DLIST_NULL) {
        anchor->tail = index;
    }
}

inline bool us_uint8_dlist_is_empty(
    UsUint8DlistAnchor *anchor
) {
    return anchor->tail == US_UINT8_DLIST_NULL;
}