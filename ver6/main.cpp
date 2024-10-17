#include <algorithm>
#include <array>
#include <initializer_list>
#include <type_traits>
#include <iostream>
#include <chrono>

constexpr int BODIES_AMNT = 100000;
constexpr float MAX_FORCE = 1.0;
constexpr float G = 6.67;
constexpr float DT = 0.01;

template <typename T, typename TNum>
struct Triple {
    TNum X = 0.f;
    TNum Y = 0.f;
    TNum Z = 0.f;

    void Init(std::array<float, 3> arr) {
        X = arr[0];
        Y = arr[1];
        Z = arr[2];
    }

    T& operator+=(const T& other) {
        X += other.X;
        Y += other.Y;
        Z += other.Z;

        return *static_cast<T*>(this);
    }
    auto operator+=(const auto&) = delete;

    T& operator-=(const T& other) {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;

        return *static_cast<T*>(this);
    }
    auto operator-=(const auto&) = delete;

    T operator+(const T& other) {
        T tmp(other);
        tmp += this;
        return tmp;
    }
    auto operator+(const auto&) = delete;

    T operator-(const T& other) {
        T tmp(other);
        tmp -= this;
        return tmp;
    }
    auto operator-(const auto&) = delete;

    Triple operator/(float number) {
        return {X / number, Y / number, Z / number};
    }
};

struct Coord : Triple<Coord, float> {
    using Triple::Init;
};

struct Velocity : Triple<Velocity, float> {
    using Triple::Init;
};

struct Acceleration : Triple<Acceleration, float> {
    using Triple::Init;
};

struct Body {
    Coord Coord;
    Velocity Vecolcity;
    float Mass = 0;
};

template <bool IsAtomic = true>
struct Force : Triple<Force<IsAtomic>, float> {
    using Triple<Force<IsAtomic>, float>::Init;

    Force& operator+=(const Force& other) {
        if constexpr (IsAtomic) {
            #pragma omp atomic
            Triple<Force<IsAtomic>, float>::X += other.X;
            #pragma omp atomic
            Triple<Force<IsAtomic>, float>::Y += other.Y;
            #pragma omp atomic
            Triple<Force<IsAtomic>, float>::Z += other.Z;
        } else {
            Triple<Force<IsAtomic>, float>::X += other.X;
            Triple<Force<IsAtomic>, float>::Y += other.Y;
            Triple<Force<IsAtomic>, float>::Z += other.Z;
        }

        return *this;
    }

    Force& operator-=(const Force& other) {
        if constexpr (IsAtomic) {
            #pragma omp atomic
            Triple<Force<IsAtomic>, float>::X -= other.X;
            #pragma omp atomic
            Triple<Force<IsAtomic>, float>::Y -= other.Y;
            #pragma omp atomic
            Triple<Force<IsAtomic>, float>::Z -= other.Z;
        } else {
            Triple<Force<IsAtomic>, float>::X -= other.X;
            Triple<Force<IsAtomic>, float>::Y -= other.Y;
            Triple<Force<IsAtomic>, float>::Z -= other.Z;
        }

        return *this;
    }

    Force& operator+=(const Force<false>& other) requires(IsAtomic == true) {
        #pragma omp atomic
        Triple<Force<IsAtomic>, float>::X += other.X;
        #pragma omp atomic
        Triple<Force<IsAtomic>, float>::Y += other.Y;
        #pragma omp atomic
        Triple<Force<IsAtomic>, float>::Z += other.Z;
    
        return *this;
    }
    Force& operator-=(const Force<false>& other) requires(IsAtomic == true) {
        #pragma omp atomic
        Triple<Force<IsAtomic>, float>::X -= other.X;
        #pragma omp atomic
        Triple<Force<IsAtomic>, float>::Y -= other.Y;
        #pragma omp atomic
        Triple<Force<IsAtomic>, float>::Z -= other.Z;

        return *this;
    }
};

using Bodies = std::array<Body, BODIES_AMNT>;
template <bool IsAtomic>
using Forces = std::array<Force<IsAtomic>, BODIES_AMNT>;

void Init(Bodies& bodies)
{
    for (int i = 0; i < BODIES_AMNT; ++i)
    {
        auto& body = bodies[i];
        body.Coord.Init({20 * (i / 20 - 20) + 10.f, 20 * (i % 20 - 10) + 10.f, 20 * (i / 20 - 20) + 10 + 20 * (i % 20 - 10) + 10.f});
        body.Vecolcity.Init({body.Coord.Y / 15, -body.Coord.X / 50, 0});
        body.Mass = 100 + i % 100;
    }
}

void CalculateForces(const Bodies& bodies, Forces<true>& forces) {
    #pragma omp parallel
    {
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
                // atomic operation
                forces[j] -= additionalForce;
            }
            forces[i] += forceiChange;
        }
    }
}

void ChangeBodiesPositions(Bodies& bodies, const Forces<true>& forces) {
    for (int i = 0; i < BODIES_AMNT; ++i) {
        // auto tid = omp_get_thread_num();
        // std::cout << tid << std::endl;

        auto& body = bodies[i];
        const auto& force = forces[i];
        Acceleration a  {force.X / body.Mass, force.Y / body.Mass, force.Z / body.Mass};

        body.Coord.X += body.Vecolcity.X * DT + a.X * DT * DT / 2;
        body.Coord.Y += body.Vecolcity.Y * DT + a.Y * DT * DT / 2;
        body.Coord.Z += body.Vecolcity.Z * DT + a.Z * DT * DT / 2;

        body.Vecolcity.X += a.X * DT;
        body.Vecolcity.Y += a.Y * DT;
        body.Vecolcity.Z += a.Z * DT;
    }
}

void ClearForces(Forces<true>& forces) {
    std::fill(forces.begin(), forces.end(), Force<>{0.f, 0.f, 0.f});
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "<ERR> Enter iter amnt" << std::endl;
        return 0;
    }

    Bodies bodies;
    Forces<true> forces;
    Init(bodies);

    int stepsAmnt = std::stoi(argv[1]);
    const auto start = std::chrono::steady_clock().now();
    for (int k = 0; k < stepsAmnt; ++k) {
        CalculateForces(bodies, forces);
        ChangeBodiesPositions(bodies, forces);
        ClearForces(forces);
    }

    const std::chrono::duration<double> duration{std::chrono::steady_clock().now() - start};
    std::cout << "Finished in " << duration.count() << std::endl;
    return 0;
}