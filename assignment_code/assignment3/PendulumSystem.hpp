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

            for (int i = 0; i < state.positions.size(); i++) {
                if (fixed[i]) {
                    positions.push_back(glm::vec3(0.f));
                    velocities.push_back(glm::vec3(0.f));
                } else {
                    positions.push_back(state.velocities[i]);
                    float m = particle_masses[i];
                    velocities.push_back((1/m) * (m*glm::vec3(0.f, -9.8f, 0.f) - 0.5f*state.velocities[i])); // 1/m * (mg + -kx')
                }
            }

            for (int s = 0; s < springs.size(); s++) {
                int i = springs[s][0];
                int j = springs[s][1];

                float m_i = particle_masses[i];
                float r_ij = springs[s][2];
                float k_ij = springs[s][3];
                glm::vec3 d_i = state.positions[i] - state.positions[j];
                glm::vec3 spring_force_i = (1/m_i) * -k_ij*(glm::length(d_i)-r_ij)*(d_i/glm::length(d_i)); // 1/m * (force sum over connected particles)
                if (!fixed[i]) {
                    velocities[i] += spring_force_i;
                }

                float m_j = particle_masses[j];
                glm::vec3 d_j = state.positions[j] - state.positions[i];
                glm::vec3 spring_force_j = (1/m_j) * -k_ij*(glm::length(d_j)-r_ij)*(d_j/glm::length(d_j)); // 1/m * (force sum over connected particles)
                if (!fixed[j]) {
                    velocities[j] += spring_force_j;
                }
            }

            return {positions, velocities};
        }

        void AddMass(float m, int is_fixed) {
            // adds particle of mass m (fixes particle if is_fixed=1)
            fixed.push_back(is_fixed);
            particle_masses.push_back(m);
        }

        void AddSpring(int i, int j, float r, float k) {
            // adds spring of rest length r and stiffness k between particles i and j
            springs.push_back(glm::vec4(i, j, r, k));
        }


        std::vector<glm::vec4> springs;
        std::vector<int> fixed; // for each index i, 1 if particle i is fixed, else 0
        std::vector<float> particle_masses; // for each index i, contains particle i's mass
};
}  // namespace GLOO

#endif