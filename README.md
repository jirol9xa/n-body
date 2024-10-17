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

## Parallel version with accumulating all force differences in thread local vars, making only 2*N atomic operations (ver7)

    Forces<false> forcejChange;
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < BODIES_AMNT; ++i) {
            // forces[i] is the same on every inner loop iter, so change it
            // only after whole loop ends
            Force<false> forceiChange;
            for (int j = i + 1; j < BODIES_AMNT; ++j) {
                const auto& first = bodies[i];
                const auto& second = bodies[j];

                float dx = second.Coord.X - first.Coord.X;
                float dy = second.Coord.Y - first.Coord.Y;
                float dz = second.Coord.Z - first.Coord.Z;
                float r_2 = 1 / (dx * dx + dy * dy + dz * dz);
                float r_1 = sqrt(r_2);

                float force = G * first.Mass * second.Mass * r_2;
                force = std::min(force, MAX_FORCE);

                Force<false> additionalForce { force * dx * r_1, force * dy * r_1, force * dz * r_1};

                // non atomic operation
                forceiChange += additionalForce;
                // non atomic operation
                forcejChange[j] -= additionalForce;
            }
            // atomic operation
            forces[i] += forceiChange;
        }

        for (int j = 0; j < BODIES_AMNT; ++j) {
            // atomic operation
            forces[j] += forcejChange[j];
        }

|O0|O3|
|--|--|
|12.75s|s|

# Interesting fact

## Parallel version with accumulating all force differences in one array, making only 1*N atomic operations (ver8)
|O0|O3|
|--|--|
|13.2s|s|
***Slowly than 2N alotic operations????*** Cache locality for forcesChange???