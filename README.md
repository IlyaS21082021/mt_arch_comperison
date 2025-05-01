Comparison of two architectures in multithread application: 
1, One queue for many threads   (DG_arch)
2. Many queues distributed among the threads (my_arch)

RESULT with buffer size 100000000
4 threads:
DG_arch computing time: 1700405 mcs
my_arch computing time: 1418179 mcs

7 threads:
DG_arch computing time: 2194381 mcs
my_arch computing time: 1691384 mcs

10 threads:
DG_arch computing time: 3097362 mcs
my_arch computing time: 3927116 mcs

12 threads:
DG_arch computing time: 3584725 mcs
my_arch computing time: 4699418 mcs




