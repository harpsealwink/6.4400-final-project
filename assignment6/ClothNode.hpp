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
            int fixed = 1;

            positions.push_back(glm::vec3(-1,  t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3( 1,  t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3(-1, -t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3( 1, -t,  0));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);

            positions.push_back(glm::vec3( 0, -1,  t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3( 0,  1,  t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3( 0, -1, -t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3( 0,  1, -t));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);

            positions.push_back(glm::vec3( t,  0, -1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3( t,  0,  1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3(-t,  0, -1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);
            positions.push_back(glm::vec3(-t,  0,  1));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, fixed);

            addTriangle(0, 11, 5);
            addTriangle(0, 5, 1);
            addTriangle(0, 1, 7);
            addTriangle(0, 7, 10);
            addTriangle(0, 10, 11);
            addTriangle(1, 5, 9);
            addTriangle(5, 11, 4);
            addTriangle(11, 10, 2);
            addTriangle(10, 7, 6);
            addTriangle(7, 1, 8);
            addTriangle(3, 9, 4);
            addTriangle(3, 4, 2);
            addTriangle(3, 2, 6);
            addTriangle(3, 6, 8);
            addTriangle(3, 8, 9);
            addTriangle(4, 9, 5);
            addTriangle(2, 4, 11);
            addTriangle(6, 2, 10);
            addTriangle(8, 6, 7);
            addTriangle(9, 8, 1);

            // add center node (connected with springs to all other nodes)
            positions.push_back(glm::vec3(0.f, 0.f, 0.f));
            velocities.push_back(glm::vec3(0.f, 0.f, 0.f));
            system.AddMass(0.1, 1);

            state = {positions, velocities};
            integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type);
            step_size = integration_step;
            
            for (int i = 0; i < triangles.size(); i++) { // add structural springs
                int node_idx = i/3;
                auto line_node = make_unique<SceneNode>();
                line_node->CreateComponent<ShadingComponent>(line_shader_);
                auto line_ = std::make_shared<VertexObject>();
                line_node->CreateComponent<MaterialComponent>(material);
                auto positions = make_unique<PositionArray>();
                auto indices = make_unique<IndexArray>();
                indices->push_back(0);
                indices->push_back(1);

                if (i % 3 == 2) { // if we are at third vertex of triangle
                    positions->push_back(state.positions[node_idx]);
                    positions->push_back(state.positions[node_idx+1]);
                } else {
                    positions->push_back(state.positions[node_idx]);
                    positions->push_back(state.positions[node_idx-2]);
                }
                
                line_->UpdatePositions(std::move(positions));
                line_->UpdateIndices(std::move(indices));
                auto &rc_curve = line_node->CreateComponent<RenderingComponent>(line_);
                rc_curve.SetDrawMode(DrawMode::Lines);

                // line_ptrs.push_back(line_);
                // AddChild(std::move(line_node));
                // system.AddSpring(n*i+j, n*(i+1)+j, l, k);
            }

            for (int i = 0; i < state.positions.size()-1; i++) { // add springs from center node to all other nodes
                auto line_node = make_unique<SceneNode>();
                line_node->CreateComponent<ShadingComponent>(line_shader_);
                auto line_ = std::make_shared<VertexObject>();
                line_node->CreateComponent<MaterialComponent>(material);
                auto positions = make_unique<PositionArray>();
                auto indices = make_unique<IndexArray>();

                indices->push_back(0);
                indices->push_back(1);
                positions->push_back(state.positions[state.positions.size()-1]); // center node
                positions->push_back(state.positions[i]);

                line_->UpdatePositions(std::move(positions));
                line_->UpdateIndices(std::move(indices));
                auto &rc_curve = line_node->CreateComponent<RenderingComponent>(line_);
                rc_curve.SetDrawMode(DrawMode::Lines);

                line_ptrs.push_back(line_);
                AddChild(std::move(line_node));
                system.AddSpring(state.positions.size()-1, i, l, k);
            }

            
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
                for (int i = 0; i < state.positions.size()-1; i++) { // update central springs
                    auto line = line_ptrs[i];
                    auto line_positions = make_unique<PositionArray>();
                    auto line_indices = make_unique<IndexArray>();
                    line_positions->push_back(state.positions[state.positions.size()-1]);
                    line_positions->push_back(state.positions[i]);
                    line_indices->push_back(0);
                    line_indices->push_back(1);
                    line->UpdatePositions(std::move(line_positions));
                    line->UpdateIndices(std::move(line_indices));
                }
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

        void addTriangle(int a, int b, int c)
        {
            triangles.push_back(a);
            triangles.push_back(b);
            triangles.push_back(c);
        }

        std::vector<SceneNode*> sphere_ptrs;
        std::vector<SceneNode*> line_node_ptrs;
        std::vector<std::shared_ptr<VertexObject>> line_ptrs;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> velocities;  
        std::vector<int> triangles;  
        ParticleState state;
        PendulumSystem system;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator; 
        float l = 0.5; // spring rest length 
        int k = 10; // spring constant
        float step_size;
};
}  // namespace GLOO

#endif

