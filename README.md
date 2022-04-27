# sundaywalkers
This is project is an exercise on the use of mutexes. 
This is an example of poor use of mutexes, as only one mutex is created and shared by 1000 threads.
A better imple,mntaion would have been to create an array of mutexes with as many mutexes as there are threads. 
Each thread would then have its own mutex.

the current implemetaion is very slow if a time delay before each lock is set at about 0.1 microsecs.
It deadlocks at any time delay above 0.1 microsecs.
