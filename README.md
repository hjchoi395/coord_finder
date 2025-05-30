# Coordinate Lookup Cache System

## Project Overview
This project implements an **in-memory cache system** designed for **high-speed retrieval of strategic coordinate data** in military environments. By storing frequently accessed coordinates in RAM, the system minimizes access to slower storage or database backends, enabling rapid response times during operations.

- **Language**: C  
- **Platform**: Windows (VSCode + PowerShell)  
- **Data Format**: Key-Value (e.g., location name → coordinate)  
- **Purpose**: Improve access latency for mission-critical tactical data

---

## Core Components

### 1. Cache Module (`cache.c / cache.h`)
- LRU (Least Recently Used) replacement policy  
- TTL (Time-To-Live) expiration logic  
- Cache hit/miss detection with fallback to DB  
- Persistent storage: `save_cache()` and `load_cache()`  
- Request count tracking (log feature in progress)

### 2. Database Module (`db.c / db.h`)
- Key-value mock database storing coordinates  
- `db_lookup()` provides fallback values for cache misses  
- Simple file-based implementation (can be extended)

### 3. Main Program (`main.c`)
- Interactive CLI for lookup requests  
- Follows the flow: **Check cache → Fallback to DB → Update cache**  
- Loads saved cache on startup  
- High-resolution timer using nanoseconds for profiling

---

## Build & Run Instructions

### 1. Compile
- gcc -o coord_finder main.c cache.c db.c

### 2. Run(Windows_Powershell)
- ./coord_finder.exe

### 3. Run(Linux, MacOS)
- ./coord_finder

---

## Project Structure

coord_finder/
├── main.c           # Main execution logic
├── cache.c/.h       # Cache logic (LRU, TTL, disk persistence)
├── db.c/.h          # DB logic (mock data source)
├── cache.dat        # Saved cache data
├── README.md




