#!/bin/sh
set -e

# build the program
gcc -std=c11 -D_POSIX_C_SOURCE=200809L -O2 -o coord_finder main.c cache.c db.c counter.c

# start from a clean state
rm -f cache_state.db counter_state.db out.txt

# pre-populate cache with a valid entry for hq1 and an expired entry for TTL test
now=$(date +%s)
printf '%s hq1 37.618367 127.496473\n0 oldkey oldvalue\n' "$now" > cache_state.db

# run about 50 lookups: 25 hits (hq1) and 25 misses (badkey)
{
    for i in $(seq 1 25); do
        echo hq1
    done
    for i in $(seq 1 25); do
        echo badkey
    done
    echo exit
} | ./coord_finder > out.txt

# compute average times in nanoseconds
hit_avg=$(grep 'CACHE HIT' out.txt | sed 's/.*(\([0-9]*\) ns).*/\1/' | awk '{sum+=$1} END {if (NR>0) printf "%.0f", sum/NR; else print 0}')
miss_avg=$(grep -E 'CACHE MISS|NOT FOUND' out.txt | sed 's/.*(\([0-9]*\) ns).*/\1/' | awk '{sum+=$1} END {if (NR>0) printf "%.0f", sum/NR; else print 0}')

printf 'Average hit time: %s ns\n' "$hit_avg"
printf 'Average miss time: %s ns\n' "$miss_avg"

# verify counter increments
hq1_count=$(grep '^hq1 ' counter_state.db | awk '{print $2}')
bad_count=$(grep '^badkey ' counter_state.db | awk '{print $2}')

if [ "$hq1_count" = "25" ] && [ "$bad_count" = "25" ]; then
  echo "Counter increments test: PASS"
else
  echo "Counter increments test: FAIL (hq1=$hq1_count badkey=$bad_count)"
  exit 1
fi

# verify TTL expiration - oldkey should not survive
if grep -q 'oldkey' cache_state.db; then
  echo "TTL expiration test: FAIL"
  exit 1
else
  echo "TTL expiration test: PASS"
fi

rm -f out.txt

echo "All tests passed."
