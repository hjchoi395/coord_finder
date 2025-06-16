# Tests

Run `./tests/run_tests.sh` from the repository root. The script builds the
program, performs about 50 lookups (half hits and half misses), and prints the
average latency for cache hits vs misses. It also verifies that counters
increment for every lookup and that expired cache entries are not reloaded.

```sh
./tests/run_tests.sh
```
