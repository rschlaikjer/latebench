# latebench
### Memcached latency benchmark tool

latebench is a very simple tool for gathering multi-get latency statistics
for memcached servers. latebench will optionally warm up the memcached server
with a normal distribution of values in a given size range, and then collect and
output the time taken to execute a given number of GET requests. Latebench is
built to run a number of iterations for multiple sizes of multi-get, and save
the timing stats for each to a named file.

### Options

- maxkeys - the number of keys that latebench should load into the server
- getsize - the maximum number of keys per get request
- stepsize - the number of keys per request to increment by each round
- iterations - the number of get requests to perform at a given get size
- threads - the number of concurrent client processes to run
- payload - the maximum size (in bytes) of values to store when warming up
- warmup - specify this to have latebench warm up the server before testing

### Dealing with output

Running a set of tests with latebench will output a handful of newline separated
timing stats. plot.r is a simple script that will take a number of files and
plot a cumulative distribution.
