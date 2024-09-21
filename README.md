# Parallel optiomizations using OpenMP

Optimization will be done on base of N-bodies proble solution using naive O(N^2) alogorithm.

Will measure result for 100`000 bodies.

## Non-optimized version
Single thread programm

|O0|O3|
|--|--|
|88.6|17.6s|

## Parallel for changing bodies positions and cleaning force's array in parallel
That operations don't need any synchronization, so it's so easy to parallelize

|O0|O3|
|--|--|
|89.0|17.8s|

Got same result -> No need to optimize that piece of code in ver3


## Parallel for changing forces arr with critical section mechanism

    #pragma omp critical
    {
        forces[i] += additionalForce;
        forces[j] -= additionalForce;
    }

|O0|O3|
|--|--|
|140|infinit time =)|

## Parallel with atomics

    #pragma omp atomic
    X += other.X;
    #pragma omp atomic
    Y += other.Y;
    #pragma omp atomic
    Z += other.Z;

|O0|O3|
|--|--|
|24|5.6|

