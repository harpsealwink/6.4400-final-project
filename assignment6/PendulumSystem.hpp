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
                }
                else {
                    positions.push_back(state.velocities[i]);
                    float m = masses_[i];
                    glm::vec3 drag_force = -b_ * state.velocities[i];
                    glm::vec3 pressure_force = normals_[i]/2.f * nRT_ / volume_; // PV = nRT --> F = A*P = A*nRT/V
                    velocities.push_back(g_ + (drag_force + pressure_force)/m); // 1/m * (mg + -kx')
                }
            }

            for (size_t s = 0; s < springs_.size(); s++) {
                int i = springs_[s][0];
                int j = springs_[s][1];

                float m_i = masses_[i];
                float r_ij = springs_[s][2];
                float k_ij = springs_[s][3];
                glm::vec3 d_i = state.positions[i] - state.positions[j];
                glm::vec3 spring_force_i = (1 / m_i) * -k_ij * (glm::length(d_i) - r_ij) * (d_i / glm::length(d_i)); // 1/m * (force sum over connected particles)
                if (!fixed_[i]) {
                    velocities[i] += spring_force_i;
                }

                float m_j = masses_[j];
                glm::vec3 d_j = state.positions[j] - state.positions[i];
                glm::vec3 spring_force_j = (1 / m_j) * -k_ij * (glm::length(d_j) - r_ij) * (d_j / glm::length(d_j)); // 1/m * (force sum over connected particles)
                if (!fixed_[j]) {
                    velocities[j] += spring_force_j;
                }
            }
            
            return { positions, velocities };
        }

        void AddMass(float m, bool is_fixed) {
            // adds particle of mass m (fixes particle if is_fixed=true)
            masses_.push_back(m);
            fixed_.push_back(is_fixed);
        }

        void AddSpring(int i, int j, float r, float k) {
            // adds spring of rest length r and stiffness k between particles i and j
            springs_.push_back(glm::vec4(i, j, r, k));
        }

        void FixMass(int i, bool is_fixed) {
            fixed_[i] = is_fixed;
        }

        glm::vec2 GetMass(int i) {
            return glm::vec2(masses_[i], fixed_[i]);
        }

        glm::vec4 GetSpring(int i) {
            return springs_[i];
        }

        void SetTriangles(std::vector<glm::vec3> triangles) {
            triangles_ = triangles;
        }

        void SetNormals(std::vector<glm::vec3> normals) {
            normals_ = normals;
        }

        void SetVolume(float volume) {
            volume_ = volume;
        }

    private:
        std::vector<glm::vec4> springs_;
        std::vector<bool> fixed_; // for each index i, true if particle i is fixed, else false
        std::vector<float> masses_; // for each index i, contains particle i's mass
        std::vector<glm::vec3> triangles_;
        std::vector<glm::vec3> normals_;
        float volume_;
        const glm::vec3 g_ = glm::vec3(0.f, -9.8f, 0.f);
        const float b_ = 0.0001f; // drag constant
        const float nRT_ = 2.0f; // pressure constant
    };
}  // namespace GLOO

#endif