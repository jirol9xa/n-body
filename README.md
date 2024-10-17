# Parallel optiomizations using OpenMP

Note: For compilation used /opt/homebrew/opt/llvm@19/bin/clang++

Optimization will be done on base of N-bodies proble solution using naive O(N^2) alogorithm.

Will measure result for 100`000 bodies.

## Non-optimized version (ver1)
Single thread programm

|O0|O3|
|--|--|
|88.6s|17.6s|

## Parallel for changing bodies positions and cleaning force's array in parallel (ver2)
That operations don't need any synchronization, so it's so easy to parallelize

|O0|O3|
|--|--|
|89.0s|17.8s|

Got same result -> No need to optimize that piece of code in ver3


## Parallel for changing forces arr with critical section mechanism (ver3)

    #pragma omp critical
    {
        forces[i] += additionalForce;
        forces[j] -= additionalForce;
    }

|O0|O3|
|--|--|
|infinit time =)|140s|

## Parallel with atomics (ver4)

    struct Force : Triple<Force, float> {
        using Triple::Init;

        Force& operator+=(const Force& other) {
            #pragma omp atomic
            X += other.X;
            #pragma omp atomic
            Y += other.Y;
            #pragma omp atomic
            Z += other.Z;

            return *this;
        }
    };

|O0|O3|
|--|--|
|23.7s|s|

## Parallel version with alignas(64) for Forces structure (ver5)
    struct alignas(16) Force

|O0|O3|
|--|--|
|27.2s|s|

## Parallel version with accumulating force difference in non thread local var avoiding ~ 50% of atomic increments (ver6)
|O0|O3|
|--|--|
|17.85s|s|