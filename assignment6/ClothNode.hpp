#ifndef CLOTH_NODE_H_
#define CLOTH_NODE_H_

#include "PendulumSystem.hpp"
#include "IntegratorFactory.hpp"
#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/InputManager.hpp"
#include <cstdlib>
#include <glm/gtx/string_cast.hpp>


namespace GLOO {
class ClothNode : public SceneNode {
    public:
        ClothNode(IntegratorType integrator_type, float integration_step) {
            auto material = std::make_shared<Material>(glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.1f, 0.1f, 0.1f), 20.0f); 
            auto line_shader_ = std::make_shared<SimpleShader>();
            auto shader_ = std::make_shared<PhongShader>();
            std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(0.03f, 25, 25);

            // create 12 vertices of a icosahedron
            float t = 0.5;

            positions.push_back(glm::vec3(-1,  t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3( 1,  t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3(-1, -t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3( 1, -t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);

            positions.push_back(glm::vec3( 0, -1,  t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3( 0,  1,  t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3( 0, -1, -t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3( 0,  1, -t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);

            positions.push_back(glm::vec3( t,  0, -1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3( t,  0,  1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3(-t,  0, -1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);
            positions.push_back(glm::vec3(-t,  0,  1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);


            state = {positions, velocities};
            integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type);
            step_size = integration_step;
            
            // for (int i = 0; i < n; i++) { // add structural springs
            //     for (int j = 0; j < n; j++) {
            //         if (i+1 < n) { // vertical springs
            //             auto line_node = make_unique<SceneNode>();
            //             line_node->CreateComponent<ShadingComponent>(line_shader_);
            //             auto line_ = std::make_shared<VertexObject>();
            //             line_node->CreateComponent<MaterialComponent>(material);

            //             auto positions = make_unique<PositionArray>();
            //             positions->push_back(state.positions[n*i+j]);
            //             positions->push_back(state.positions[n*(i+1)+j]);
            //             auto indices = make_unique<IndexArray>();
            //             indices->push_back(0);
            //             indices->push_back(1);
            //             line_->UpdatePositions(std::move(positions));
            //             line_->UpdateIndices(std::move(indices));
            //             auto &rc_curve = line_node->CreateComponent<RenderingComponent>(line_);
            //             rc_curve.SetDrawMode(DrawMode::Lines);

            //             line_ptrs.push_back(line_);
            //             // line_node_ptrs.push_back(line_node.get());
            //             AddChild(std::move(line_node));
            //             system.AddSpring(n*i+j, n*(i+1)+j, l, k);
            //         }
            //         if (j+1 < n) { // horizontal springs
            //             auto line_node = make_unique<SceneNode>();
            //             line_node->CreateComponent<ShadingComponent>(line_shader_);
            //             auto line_ = std::make_shared<VertexObject>();
            //             line_node->CreateComponent<MaterialComponent>(material);

            //             auto positions = make_unique<PositionArray>();
            //             positions->push_back(state.positions[n*i+j]);
            //             positions->push_back(state.positions[n*i+j+1]);
            //             auto indices = make_unique<IndexArray>();
            //             indices->push_back(0);
            //             indices->push_back(1);
            //             line_->UpdatePositions(std::move(positions));
            //             line_->UpdateIndices(std::move(indices));
            //             auto &rc_curve = line_node->CreateComponent<RenderingComponent>(line_);
            //             rc_curve.SetDrawMode(DrawMode::Lines);

            //             line_ptrs.push_back(line_);
            //             // line_node_ptrs.push_back(line_node.get());
            //             AddChild(std::move(line_node));
            //             system.AddSpring(n*i+j, n*i+j+1, l, k);
            //         }
            //     }
            // }
            
            // for (int i = 0; i < n; i++) { // add structural springs
            //     for (int j = 0; j < n; j++) {
            //         if (i+1 < n && j+1 < n) { // diagonal down & right springs
            //             system.AddSpring(n*i+j, n*(i+1)+j+1, l, k);
            //         }
            //         if (i+1 < n && j-1 > -1) { // diagonal down & left springs
            //             system.AddSpring(n*i+j, n*(i+1)+j-1, l, k);
            //         }
            //     }
            // }

            // for (int i = 0; i < n; i++) { // add flex springs
            //     for (int j = 0; j < n; j++) {
            //         if (i+2 < n) { // horizontal springs
            //             system.AddSpring(n*i+j, n*(i+2)+j, l, k);
            //         }
            //         if (j+2 < n) { // vertical springs
            //             system.AddSpring(n*i+j, n*i+j+2, l, k);
            //         }
            //     }
            // }

            // render spheres
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
            bool wind = false;
            while (start_time < delta_time) {
                state = integrator->Integrate(system, state, start_time, fmin(step_size, delta_time)); // step sizes cannot be greater than time
                for (int i = 0; i < sphere_ptrs.size(); i++) {
                    sphere_ptrs[i]->GetTransform().SetPosition(state.positions[i]);
                }
                // int line_count = 0;
                // for (int i = 0; i < n; i++) { // update structural springs
                //     for (int j = 0; j < n; j++) {
                //         if (i+1 < n) { // vertical springs
                //             auto line = line_ptrs[line_count];
                //             auto line_positions = make_unique<PositionArray>();
                //             auto line_indices = make_unique<IndexArray>();
                //             line_positions->push_back(state.positions[n*i+j]);
                //             line_positions->push_back(state.positions[n*(i+1)+j]);
                //             line_indices->push_back(0);
                //             line_indices->push_back(1);
                //             line->UpdatePositions(std::move(line_positions));
                //             line->UpdateIndices(std::move(line_indices));
                //             line_count ++;
                //         }
                //         if (j+1 < n) { // horizontal springs
                //             auto line = line_ptrs[line_count];
                //             auto line_positions = make_unique<PositionArray>();
                //             auto line_indices = make_unique<IndexArray>();
                //             line_positions->push_back(state.positions[n*i+j]);
                //             line_positions->push_back(state.positions[n*i+j+1]);
                //             line_indices->push_back(0);
                //             line_indices->push_back(1);
                //             line->UpdatePositions(std::move(line_positions));
                //             line->UpdateIndices(std::move(line_indices));
                //             line_count ++;
                //         }
                //     }
                // }
                start_time += step_size;
            }

            static bool prev_released = true;
            if (InputManager::GetInstance().IsKeyPressed('R')) {
                if (prev_released) {
                    state = {positions, velocities};
                }
                prev_released = false;
            } else if (InputManager::GetInstance().IsKeyPressed('W')) {
                wind = true;
            } else {
                prev_released = true;
            }
            if (wind) {
                for (int i = 0; i < state.velocities.size(); i++) {
                    int rand_num = rand() % 20;
                    state.velocities[i] += glm::vec3(0.f, 0.f, -0.02f*rand_num); // random backwards wind force
                }
            }
        }

        void addTriangle(uint32_t a, uint32_t b, uint32_t c)
        {
            triangles.emplace_back(a);
            triangles.emplace_back(b);
            triangles.emplace_back(c);
        }

        std::vector<SceneNode*> sphere_ptrs;
        std::vector<SceneNode*> line_node_ptrs;
        std::vector<std::shared_ptr<VertexObject>> line_ptrs;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> velocities;  
        std::vector<glm::vec3> triangles;  
        ParticleState state;
        PendulumSystem system;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator; 
        int n = 5; // dimensions of cloth (number of spheres per side)
        float l = 0.11; // spring rest length 
        int k = 20; // spring constant
        float step_size;
};
}  // namespace GLOO

#endif

