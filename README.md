Coordinate Lookup Cache System

Project Overview

This project provides an in‑memory cache for rapid retrieval of frequently used coordinate data in military applications, lowering database latency and improving real‑time responsiveness.

Language: C

Platform: Windows (VS Code · PowerShell · GCC/MinGW)

Data Model: Key–value (name → coordinate)

Modules

cache.c / cache.h

LRU replacement policy

TTL‑based expiration

Disk persistence via save_cache() and load_cache()

Hit/miss detection with access counters

db.c / db.h

File‑backed key–value store

db_lookup() called on cache miss

main.c

Command‑line interface for lookups

Flow: check cache → DB fallback → update cache

Restores saved cache on startup

Nanosecond‑precision timing for profiling
