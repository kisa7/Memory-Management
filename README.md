# Memory-Management
dynamic storage allocator for C programs


Estimators: The driver program summarizes the performance of your allocator by computing a performance index, P , 
which is a weighted sum of the space utilization and throughput
P = wU + (1 − w)min(1, T/Tlibc)
where U is your space utilization, T is your throughput, and Tlibc is the estimated throughput of libc 
malloc on the system on the default traces. The performance index favors space utilization over throughput, 
with a default of w = 0.6.
Observing that both memory and CPU cycles are expensive system resources, adopt was adopted this formula to encourage
balanced optimization of both memory utilization and throughput. Ideally, the performance index will reach 
P = w + (1 − w) = 1 or 100%. Since each metric will contribute at most w and 1 − w to the performance index, 
respectively, you should not go to extremes to optimize either the memory utilization or the throughput only. 
To receive a good score, must be achieved a balance between utilization and throughput.

Implementation: I implemented my own vision of segregated lists, I write the address of the 
 next block from current free list in data segment of block(block is free, so who cares what is inside).
 And I got array of lists with the classes of same size, power of 2. For this implementation I achieved 64% efficiency.
