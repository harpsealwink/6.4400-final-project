#ifndef CIRCLE_NODE_H_
#define CIRCLE_NODE_H_

#include "SimpleSystem.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include <glm/gtx/string_cast.hpp>


namespace GLOO {
class CircleNode : public SceneNode {
    public:
        CircleNode(IntegratorType integrator_type, float integration_step) {
            positions.push_back(glm::vec3(-0.4f, 0.f, 0.f));
            velocities.push_back(glm::vec3(0.f));
            state = {positions, velocities};
            integrator = IntegratorFactory::CreateIntegrator<SimpleSystem, ParticleState>(integrator_type);
            step_size = integration_step;

            // make sphere, save sphere pointer as variable
            auto sphere_node = make_unique<SceneNode>();
            std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(0.1f, 25, 25);
            auto shader_ = std::make_shared<PhongShader>();
            auto material = std::make_shared<Material>(glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.1f, 0.1f, 0.1f), 20.0f); 
            sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
            sphere_node->CreateComponent<MaterialComponent>(material);
            sphere_node->CreateComponent<ShadingComponent>(shader_);
            sphere_ptr = sphere_node.get();
            AddChild(std::move(sphere_node));
        };

        void Update(double delta_time) {
            float start_time = 0;
            while (start_time < delta_time) {
                state = integrator->Integrate(system, state, start_time, fmin(step_size, delta_time)); // step sizes cannot be greater than time
                sphere_ptr->GetTransform().SetPosition(state.positions[0]);
                start_time += step_size;
            }
        }

        SceneNode* sphere_ptr;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> velocities;  
        ParticleState state;
        SimpleSystem system;
        std::unique_ptr<IntegratorBase<SimpleSystem, ParticleState>> integrator; 
        float step_size;
};
}  // namespace GLOO

#endif

