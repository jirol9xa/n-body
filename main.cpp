#include <array>
#include <initializer_list>
#include <type_traits>

constexpr int BODIES_AMNT = 100;
constexpr float MAX_FORCE = 1.0;
constexpr float G = 6.67;
constexpr float DT = 0.01;


template <typename T>
struct Triple {
    float X = 0;
    float Y = 0;
    float Z = 0;

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

    Triple operator/(float number) {
        return {X / number, Y / number, Z / number};
    }
};

struct Coord : Triple<Coord> {
    using Triple::Init;
};

struct Velocity : Triple<Velocity> {
    using Triple::Init;
};

struct Acceleration : Triple<Acceleration> {
    using Triple::Init;
};

struct Body {
    Coord Coord;
    Velocity Vecolcity;
    float Mass = 0;
};

struct Force : Triple<Force> {
    using Triple::Init;
};

using Bodies = std::array<Body, BODIES_AMNT>;
using Forces = std::array<Force, BODIES_AMNT>;

void Init(Bodies& bodies, Forces& forces)
{
    for (int i = 0; i < BODIES_AMNT; ++i)
    {
        auto& body = bodies[i];
        body.Coord.Init({20 * (i / 20 - 20) + 10.f, 20 * (i % 20 - 10) + 10.f, 20 * (i / 20 - 20) + 10 + 20 * (i % 20 - 10) + 10.f});
        body.Vecolcity.Init({body.Coord.Y / 15, -body.Coord.X / 50, 0});
        body.Mass = 100 + i % 100;
    }
}

void CalculateForces(const Bodies& bodies, Forces& forces) {
    for (int i = 0; i < BODIES_AMNT; ++i) {
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

            Force additionalForce { force * dx * r_1, force * dy * r_1, force * dz * r_1};

            forces[i] += additionalForce;
            forces[j] -= additionalForce;
        }
    }
}

void ChangeBodiesPositions(Bodies& bodies, const Forces& forces) {
    for (int i = 0; i < BODIES_AMNT; ++i) {
        float a = forces[i]
    }
}

int main() {
    Bodies bodies;
    Forces forces;
    for (int i = 0; i < BODIES_AMNT; ++i) {

    }


    return 0;
}