#ifndef PENDULUM_SYSTEM_H_
#define PENDULUM_SYSTEM_H_

#include "ParticleSystemBase.hpp"
#include <glm/gtx/string_cast.hpp>


namespace GLOO {
class PendulumSystem : public ParticleSystemBase {
    public:
        ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
            std::vector<glm::vec3> positions;            
            std::vector<glm::vec3> velocities;

            for (size_t i = 0; i < state.positions.size(); i++) {
                if (fixed_[i]) {
                    positions.push_back(glm::vec3(0.f));
                    velocities.push_back(glm::vec3(0.f));
                } else {
                    positions.push_back(state.velocities[i]);
                    float m = particle_masses_[i];
                    velocities.push_back((1/m) * (m*glm::vec3(0.f, -9.8f, 0.f) - 0.5f*state.velocities[i])); // 1/m * (mg + -kx')
                }
            }

            for (size_t s = 0; s < springs_.size(); s++) {
                int i = springs_[s][0];
                int j = springs_[s][1];

                float m_i = particle_masses_[i];
                float r_ij = springs_[s][2];
                float k_ij = springs_[s][3];
                glm::vec3 d_i = state.positions[i] - state.positions[j];
                glm::vec3 spring_force_i = (1/m_i) * -k_ij*(glm::length(d_i)-r_ij)*(d_i/glm::length(d_i)); // 1/m * (force sum over connected particles)
                if (!fixed_[i]) {
                    velocities[i] += spring_force_i;
                }

                float m_j = particle_masses_[j];
                glm::vec3 d_j = state.positions[j] - state.positions[i];
                glm::vec3 spring_force_j = (1/m_j) * -k_ij*(glm::length(d_j)-r_ij)*(d_j/glm::length(d_j)); // 1/m * (force sum over connected particles)
                if (!fixed_[j]) {
                    velocities[j] += spring_force_j;
                }
            }

            return {positions, velocities};
        }

        void AddMass(float m, bool is_fixed) {
            // adds particle of mass m (fixes particle if is_fixed=true)
            fixed_.push_back(is_fixed);
            particle_masses_.push_back(m);
        }

        void AddSpring(int i, int j, float r, float k) {
            // adds spring of rest length r and stiffness k between particles i and j
            springs_.push_back(glm::vec4(i, j, r, k));
        }


        std::vector<glm::vec4> springs_;
        std::vector<bool> fixed_; // for each index i, true if particle i is fixed, else false
        std::vector<float> particle_masses_; // for each index i, contains particle i's mass
};
}  // namespace GLOO

#endif