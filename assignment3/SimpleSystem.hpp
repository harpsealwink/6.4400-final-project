#ifndef SIMPLE_SYSTEM_H_
#define SIMPLE_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class SimpleSystem : public ParticleSystemBase {
    public:
        ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> velocities;
            positions.push_back(glm::vec3(-state.positions[0][1], state.positions[0][0], 0.f));
            velocities.push_back(glm::vec3(0.f));
            return {positions, velocities};
        }
};
}  // namespace GLOO

#endif