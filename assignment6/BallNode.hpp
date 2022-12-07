#ifndef BALL_NODE_H_
#define BALL_NODE_H_

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
    class BallNode : public SceneNode {
    public:
        BallNode(IntegratorType integrator_type, float integration_step) {
            auto material = std::make_shared<Material>(glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.6f, 0.2f, 0.25f), glm::vec3(0.1f, 0.1f, 0.1f), 20.0f);
            auto line_shader_ = std::make_shared<SimpleShader>();
            auto shader_ = std::make_shared<PhongShader>();
            std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(0.03f, 25, 25);

            // create 12 vertices of a icosahedron
            const float t = (1.f + sqrt(5.f)) / 2.f;
            float scale = .1;
            glm::vec3 center(1.f, 0.f, 0.f);
            bool fixed = false;

            positions_.push_back(glm::vec3(-1.f, t, 0.f) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(1.f, t, 0.f) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(-1.f, -t, 0.f) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(1.f, -t, 0.f) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);

            positions_.push_back(glm::vec3(0.f, -1.f, t) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(0.f, 1.f, t) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(0.f, -1.f, -t) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(0.f, 1.f, -t) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);

            positions_.push_back(glm::vec3(t, 0.f, -1) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(t, 0.f, 1.f) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(-t, 0.f, -1) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);
            positions_.push_back(glm::vec3(-t, 0.f, 1.f) * scale + center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, fixed);

            AddTriangle(0, 11, 5);
            AddTriangle(0, 5, 1);
            AddTriangle(0, 1, 7);
            AddTriangle(0, 7, 10);
            AddTriangle(0, 10, 11);
            AddTriangle(1, 5, 9);
            AddTriangle(5, 11, 4);
            AddTriangle(11, 10, 2);
            AddTriangle(10, 7, 6);
            AddTriangle(7, 1, 8);
            AddTriangle(3, 9, 4);
            AddTriangle(3, 4, 2);
            AddTriangle(3, 2, 6);
            AddTriangle(3, 6, 8);
            AddTriangle(3, 8, 9);
            AddTriangle(4, 9, 5);
            AddTriangle(2, 4, 11);
            AddTriangle(6, 2, 10);
            AddTriangle(8, 6, 7);
            AddTriangle(9, 8, 1);

            // add center node (connected with springs to all other nodes)
            positions_.push_back(center);
            velocities_.push_back(glm::vec3(0.f, 0.f, 0.f));
            system_.AddMass(0.1, 1);

            for (int i = 0; i < triangles_.size(); i++) { // add surface springs
                auto line_node = make_unique<SceneNode>();
                line_node->CreateComponent<ShadingComponent>(line_shader_);
                auto line_ = std::make_shared<VertexObject>();
                line_node->CreateComponent<MaterialComponent>(material);
                auto positions = make_unique<PositionArray>();
                auto indices = make_unique<IndexArray>();
                indices->push_back(0);
                indices->push_back(1);
                indices->push_back(1);
                indices->push_back(2);
                indices->push_back(2);
                indices->push_back(0);

                positions->push_back(positions_[triangles_[i][0]]);
                positions->push_back(positions_[triangles_[i][1]]);
                positions->push_back(positions_[triangles_[i][2]]);

                line_->UpdatePositions(std::move(positions));
                line_->UpdateIndices(std::move(indices));
                auto& rc_curve = line_node->CreateComponent<RenderingComponent>(line_);
                rc_curve.SetDrawMode(DrawMode::Lines);
                surface_line_ptrs_.push_back(line_);
                AddChild(std::move(line_node));
            }



            for (int i = 0; i < positions_.size() - 1; i++) { // add radial springs
                auto line_node = make_unique<SceneNode>();
                line_node->CreateComponent<ShadingComponent>(line_shader_);
                auto line_ = std::make_shared<VertexObject>();
                line_node->CreateComponent<MaterialComponent>(material);
                auto positions = make_unique<PositionArray>();
                auto indices = make_unique<IndexArray>();

                indices->push_back(0);
                indices->push_back(1);
                positions->push_back(positions_[positions_.size() - 1]); // center node
                positions->push_back(positions_[i]);

                line_->UpdatePositions(std::move(positions));
                line_->UpdateIndices(std::move(indices));
                auto& rc_curve = line_node->CreateComponent<RenderingComponent>(line_);
                rc_curve.SetDrawMode(DrawMode::Lines);

                radial_line_ptrs_.push_back(line_);
                AddChild(std::move(line_node));
                system_.AddSpring(positions_.size() - 1, i, l, k);
            }

            // render spheres
            for (int i = 0; i < positions_.size(); i++) {
                auto sphere_node = make_unique<SceneNode>();
                sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
                sphere_node->CreateComponent<MaterialComponent>(material);
                sphere_node->CreateComponent<ShadingComponent>(shader_);
                sphere_ptrs_.push_back(sphere_node.get());
                AddChild(std::move(sphere_node));
            }

            state_ = { positions_, velocities_ };
            integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type);
            step_size_ = integration_step;
        };


        void Update(double delta_time) {
            float start_time = 0;
            bool wind = false;
            while (start_time < delta_time) {
                state_ = integrator_->Integrate(system_, state_, start_time, fmin(step_size_, delta_time)); // step sizes cannot be greater than time
                for (int i = 0; i < sphere_ptrs_.size(); i++) {
                    sphere_ptrs_[i]->GetTransform().SetPosition(state_.positions[i]);
                }
                for (int i = 0; i < state_.positions.size() - 1; i++) { // update central springs
                    auto line = radial_line_ptrs_[i];
                    auto line_positions = make_unique<PositionArray>();
                    auto line_indices = make_unique<IndexArray>();
                    line_positions->push_back(state_.positions[state_.positions.size() - 1]);
                    line_positions->push_back(state_.positions[i]);
                    line_indices->push_back(0);
                    line_indices->push_back(1);
                    line->UpdatePositions(std::move(line_positions));
                    line->UpdateIndices(std::move(line_indices));
                }
                start_time += step_size_;
            }

            static bool prev_released = true;
            if (InputManager::GetInstance().IsKeyPressed('R')) {
                if (prev_released) {
                    state_ = { positions_, velocities_ };
                }
                prev_released = false;
            }
            else if (InputManager::GetInstance().IsKeyPressed('W')) {
                wind = true;
            }
            else {
                prev_released = true;
            }
            if (wind) {
                for (int i = 0; i < state_.velocities.size(); i++) {
                    int rand_num = rand() % 20;
                    state_.velocities[i] += glm::vec3(0.f, 0.f, -0.02f * rand_num); // random backwards wind force
                }
            }
        }
    private:
        void AddVertex(glm::vec3 position, glm::vec3 velocity, float mass, bool fixed) {
            positions_.push_back(position);
            velocities_.push_back(velocity);
            system_.AddMass(mass, fixed);
        }

        void AddTriangle(int a, int b, int c) { // vertices of triangle are at indices a, b, c
            triangles_.push_back(glm::vec3(a, b, c));
        }

        std::vector<SceneNode*> sphere_ptrs_;
        //std::vector<SceneNode*> line_node_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> surface_line_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> radial_line_ptrs_;
        std::vector<glm::vec3> positions_;
        std::vector<glm::vec3> velocities_;
        std::vector<glm::vec3> triangles_;
        ParticleState state_;
        PendulumSystem system_;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
        float l = 0.5; // spring rest length 
        int k = 10; // spring constant
        float step_size_;
    };
}  // namespace GLOO

#endif

