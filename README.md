# SkyKitC Speedtests
This is a benchmark program of different functions for SkyKitC. Mainly, I use it to benchmark different angle approximations ex cosine. THIS IS *NOT* AN ACCURACY TEST, IT ONLY TESTS SPEED.

I know the code is an absolute *mess* right now, I need to clean it up... cd into this directory and ./c.sh to compile test, and then run ./test a couple times.

`test_speedcases_snoolie()` tests the speed of two different versions of a cosine approximation, and `test_speedcases()` tests between libc cos() and the cosine approximation here.
