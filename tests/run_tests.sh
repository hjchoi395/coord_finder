#!/bin/sh
set -e

# build the program
gcc -std=c11 -D_POSIX_C_SOURCE=200809L -O2 -o coord_finder main.c cache.c db.c counter.c

# start from a clean state
rm -f cache_state.db counter_state.db out.txt

# pre-populate cache with only an expired entry for TTL test
printf '0 oldkey oldvalue\n' > cache_state.db

# run lookups: hq1~hq10, cp1~cp10, point10001~point10030 twice each
keys=""
for i in $(seq 1 10); do
    keys="$keys hq$i"
done
for i in $(seq 1 10); do
    keys="$keys cp$i"
done
for i in $(seq 10001 10030); do
    keys="$keys point$i"
done

{
    for k in $keys; do
        echo "$k"
    done
    for k in $keys; do
        echo "$k"
    done
    echo exit
} | ./coord_finder > out.txt

# compute average times in nanoseconds
times=$(grep -o '[0-9]\+ ns' out.txt | awk '{print $1}')
miss_avg=$(echo "$times" | head -n 50 | awk '{sum+=$1} END {if (NR>0) printf "%.0f", sum/NR; else print 0}')
hit_avg=$(echo "$times" | tail -n 50 | awk '{sum+=$1} END {if (NR>0) printf "%.0f", sum/NR; else print 0}')

printf 'Average hit time: %s ns\n' "$hit_avg"
printf 'Average miss time: %s ns\n' "$miss_avg"

# verify counter increments - each key should have count=2
fail=0
for k in $keys; do
    cnt=$(grep "^$k " counter_state.db | awk '{print $2}')
    if [ "$cnt" != "2" ]; then
        echo "Counter mismatch for $k: $cnt"
        fail=1
        break
    fi
done

if [ $fail -eq 0 ]; then
  echo "Counter increments test: PASS"
else
  echo "Counter increments test: FAIL"
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
