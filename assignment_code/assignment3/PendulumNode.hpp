#ifndef PENDULUM_NODE_H_
#define PENDULUM_NODE_H_

#include "PendulumSystem.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include <glm/gtx/string_cast.hpp>


namespace GLOO {
class PendulumNode : public SceneNode {
    public:
        PendulumNode(IntegratorType integrator_type, float integration_step) {
            // add particles to system
            positions.push_back(glm::vec3(1.f, 1.6f, 0.f));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.8, 1);
            positions.push_back(glm::vec3(1.f, 1.55f, 0.9f));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.8, 0);
            positions.push_back(glm::vec3(1.f, 1.5f, 0.f));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.8, 0);
            positions.push_back(glm::vec3(1.f, 1.45f, -0.9f));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.8, 0);
            state = {positions, velocities};
            system.AddSpring(0, 1, 0.01, 20);
            system.AddSpring(1, 2, 0.01, 20);
            system.AddSpring(2, 3, 0.01, 20);
            
            integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type);
            step_size = integration_step;

            // render spheres
            auto shader_ = std::make_shared<PhongShader>();
            auto material = std::make_shared<Material>(glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.1f, 0.1f, 0.1f), 20.0f); 
            std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(0.05f, 25, 25);
            for (int i = 0; i < positions.size(); i++) {
                auto sphere_node = make_unique<SceneNode>();
                sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
                sphere_node->CreateComponent<MaterialComponent>(material);
                sphere_node->CreateComponent<ShadingComponent>(shader_);
                sphere_ptrs.push_back(sphere_node.get());
                AddChild(std::move(sphere_node));
            }
        };

        void Update(double delta_time) {
            float start_time = 0;
            while (start_time < delta_time) {
                state = integrator->Integrate(system, state, start_time, fmin(step_size, delta_time)); // step sizes cannot be greater than time
                for (int i = 0; i < sphere_ptrs.size(); i++) {
                    sphere_ptrs[i]->GetTransform().SetPosition(state.positions[i]);
                }
                start_time += step_size;
            }
        }

        std::vector<SceneNode*> sphere_ptrs;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> velocities;  
        ParticleState state;
        PendulumSystem system;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator; 
        float step_size;
};
}  // namespace GLOO

#endif

