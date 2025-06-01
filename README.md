# Coordinate Lookup Cache System

## Project Overview

This project implements an **in-memory cache system** designed for **high-speed retrieval of strategic coordinate data** in military environments. By storing frequently accessed coordinates in RAM, the system minimizes access to slower storage or database backends, enabling rapid response times during operations.

- **Language**: C  
- **Platform**: Windows (VSCode + PowerShell) / Linux / macOS  
- **Data Format**: Key–Value (e.g., location name → coordinate)  
- **Purpose**: Improve access latency for mission-critical tactical data  
- **Persistence**: Cache and call‐count history are saved to disk and reloaded on startup  

---

## Core Components

### 1. Cache Module (`cache.c` / `cache.h`)

- **LRU (Least Recently Used)** replacement policy  
- **TTL (Time-To-Live)** expiration logic (14 days by default)  
- **Cache hit/miss detection** with fallback to DB  
- **Persistent storage**:  
  - `save_cache()` writes valid entries to `cache_state.db` upon exit  
  - `load_cache()` restores entries (if not expired) on startup  
- **Interactive “cache” command** to display current in-memory cache contents  

### 2. Database Module (`db.c` / `db.h`)

- **File-based key–value store** (`coordinates.db`) as a mock database  
- `db_lookup()` scans the file on cache misses and returns the coordinate if found  
- Seamless fallback so that any valid key not in cache will be retrieved from DB  

### 3. Call-Count Module (`counter.c` / `counter.h`)

- **Tracks how many times** each key is requested (regardless of hit or miss)  
- Implements a **singly linked list** (key → count) in memory  
- **Persistent storage**:  
  - `save_counter()` writes all key–count pairs to `counter_state.db` upon exit  
  - `load_counter()` restores counts on startup  
- **Interactive “counter” command** to display the full call-count history in memory  

### 4. Main Program (`main.c`)

- **Interactive CLI** for lookup requests and status commands  
- Workflow for each input:  
  1. **Increment call count** for the given key  
  2. **Check cache** via `cache_lookup()`  
     - If hit: print “CACHE HIT” + value + lookup time  
     - If miss: call `db_lookup()`, then `cache_insert()` on success, or “NOT FOUND” if missing  
  3. **Print latency** (nanoseconds) for each lookup  
- **Built-in commands**:  
  - `cache` → display the in-memory cache contents  
  - `counter` → display the in-memory call-count history  
  - `exit` → save state to disk and terminate  
- On startup:  
  1. `init_cache()` + `load_cache()` → restore previous cache entries  
  2. `counter_init()` + `load_counter()` → restore previous call counts  
- On exit:  
  1. `save_cache()` + print final cache state  
  2. `save_counter()` + print final call counts  
  3. Free all allocated memory  

---
## Build & Run Instructions

### Windows(Powershell)
gcc -std=c11 -O2 -o coord_finder.exe main.c cache.c db.c counter.c
./coord_finder.exe

### Linux/MacOS
gcc -std=c11 -O2 -o coord_finder main.c cache.c db.c counter.c
./coord_finder

---
## Project Structures
```text
coord_finder/
├── main.c              # Main execution logic (CLI + workflow)
├── cache.c             # Cache logic: LRU, TTL, persistence
├── cache.h             # Cache interface
├── db.c                # DB fallback logic (file-based lookup)
├── db.h                # DB interface
├── counter.c           # Call-count logic: in-memory + persistence
├── counter.h           # Counter interface
├── coordinates.db      # (Optional) Mock database file: <key> <value> per line
├── cache_state.db      # Generated on exit: saved cache entries
├── counter_state.db    # Generated on exit: saved call counts
├── README.md           # This file (project overview & instructions)
└── Makefile            # (Optional) Build automation script
