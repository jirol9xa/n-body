#include <array>

constexpr int BODIES_AMNT = 100;
constexpr float MAX_FORCE = 1.0;
constexpr float G = 6.67;
constexpr float DT = 0.01;

struct Triple {
    float X = 0;
    float Y = 0;
    float Z = 0;

    Triple& operator+=(const Triple& other) {
        X += other.X;
        Y += other.Y;
        Z += other.Z;

        return *this;
    }
    Triple& operator-=(const Triple& other) {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;

        return *this;
    }

    Triple operator/(float number) {
        return {X / number, Y / number, Z / number};
    }
};

struct Coord : Triple {};

struct Velocity : Triple {};

struct Acceleration : Triple {};

struct Body {
    Coord XYZ;
    Velocity V;

    float Mass = 0;
};

struct Force : Triple {
    Force& operator+=(const Force& other) {
        Triple::operator+=(other);
        return *this;
    }
    Force& operator-=(const Force& other) {
        Triple::operator-=(other);
        return *this;
    }

    Force operator+=(const auto&) = delete;
    Force operator-=(const auto&) = delete;
};

using Bodies = std::array<Body, BODIES_AMNT>;
using Forces = std::array<Force, BODIES_AMNT>;

void Init(Bodies& bodies, Forces& forces)
{
    for (int i = 0; i < BODIES_AMNT; ++i)
    {
        auto& body = bodies[i];
        body.X = 20 * (i / 20 - 20) + 10;
        body.Y = 20 * (i % 20 - 10) + 10;
        body.Z = body.X + body.Y;
        body.Vx = body.Y / 15;
        body.Vy = -body.X / 50;
        body.Mass = 100 + i % 100;
    }
}

void CalculateForces(const Bodies& bodies, Forces& forces) {
    for (int i = 0; i < BODIES_AMNT; ++i) {
        for (int j = i + 1; j < BODIES_AMNT; ++j) {
            const auto& first = bodies[i];
            const auto& second = bodies[j];

            float dx = second.X - first.X;
            float dy = second.Y - first.Y;
            float dz = second.Z - first.Z;
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