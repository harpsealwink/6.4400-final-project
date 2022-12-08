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
            integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type);
            step_size_ = integration_step;

            auto material = std::make_shared<Material>(
                glm::vec3(0.6f, 0.2f, 0.25f),
                glm::vec3(0.6f, 0.2f, 0.25f),
                glm::vec3(0.1f, 0.1f, 0.1f), 20.0f);
            auto line_shader = std::make_shared<SimpleShader>();
            auto shader = std::make_shared<PhongShader>();
            std::shared_ptr<VertexObject> sphere_mesh = PrimitiveFactory::CreateSphere(0.03f, 25, 25);

            // initialize icosahedron
            glm::vec3 center(0.f, 0.f, 0.f);
            float scale = 1.f;
            glm::vec3 velocity(0.f, 0.f, 0.f);
            InitIcosahedron(center, scale, velocity);

            // TODO: subdivision of initial icosahedron into icosphere

            // add surface springs
            for (size_t i = 0; i < triangles_.size(); i++) { 
                auto line_node = make_unique<SceneNode>();
                line_node->CreateComponent<MaterialComponent>(material);
                line_node->CreateComponent<ShadingComponent>(line_shader);
                
                auto positions = make_unique<PositionArray>();
                auto indices = make_unique<IndexArray>();
                auto line = std::make_shared<VertexObject>();
                positions->push_back(positions_[triangles_[i][0]]);
                positions->push_back(positions_[triangles_[i][1]]);
                positions->push_back(positions_[triangles_[i][2]]);
                indices->push_back(0);
                indices->push_back(1);
                indices->push_back(1);
                indices->push_back(2);
                indices->push_back(2);
                indices->push_back(0);
                line->UpdatePositions(std::move(positions));
                line->UpdateIndices(std::move(indices));

                auto& rc_curve = line_node->CreateComponent<RenderingComponent>(line);
                rc_curve.SetDrawMode(DrawMode::Lines);
                surface_line_ptrs_.push_back(line);
                AddChild(std::move(line_node));
                system_.AddSpring(triangles_[i][0], triangles_[i][1], surface_l_, surface_k_);
                system_.AddSpring(triangles_[i][1], triangles_[i][2], surface_l_, surface_k_);                
                system_.AddSpring(triangles_[i][2], triangles_[i][0], surface_l_, surface_k_);
            }

            // add radial springs
            for (size_t i = 0; i < positions_.size() - 1; i++) {
                auto line_node = make_unique<SceneNode>();
                line_node->CreateComponent<MaterialComponent>(material);
                line_node->CreateComponent<ShadingComponent>(line_shader);
                
                auto positions = make_unique<PositionArray>();
                auto indices = make_unique<IndexArray>();
                auto line = std::make_shared<VertexObject>();
                positions->push_back(positions_[positions_.size() - 1]); // center node
                positions->push_back(positions_[i]);
                indices->push_back(0);
                indices->push_back(1);
                line->UpdatePositions(std::move(positions));
                line->UpdateIndices(std::move(indices));

                auto& rc_curve = line_node->CreateComponent<RenderingComponent>(line);
                rc_curve.SetDrawMode(DrawMode::Lines);
                radial_line_ptrs_.push_back(line);
                AddChild(std::move(line_node));
                system_.AddSpring(positions_.size() - 1, i, radial_l_, radial_k_);
            }

            // render spheres
            for (size_t i = 0; i < positions_.size(); i++) {
                auto sphere_node = make_unique<SceneNode>();
                sphere_node->CreateComponent<MaterialComponent>(material);
                sphere_node->CreateComponent<ShadingComponent>(shader);
                sphere_node->CreateComponent<RenderingComponent>(sphere_mesh);
                sphere_node_ptrs_.push_back(sphere_node.get());
                AddChild(std::move(sphere_node));
            }

            state_ = { positions_, velocities_ };
        };

        void Update(double delta_time) {
            double start_time = 0.0;
            while (start_time < delta_time) {
                state_ = integrator_->Integrate(system_, state_, start_time, fmin(step_size_, delta_time)); // step sizes cannot be greater than time
                for (size_t i = 0; i < sphere_node_ptrs_.size(); i++) {
                    sphere_node_ptrs_[i]->GetTransform().SetPosition(state_.positions[i]);
                }

                // update radial springs
                for (size_t i = 0; i < state_.positions.size() - 1; i++) {
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

                // update surface springs
                for (size_t i = 0; i < triangles_.size(); i++) {
                    auto line = surface_line_ptrs_[i];
                    auto line_positions = make_unique<PositionArray>();
                    auto line_indices = make_unique<IndexArray>();
                    line_positions->push_back(state_.positions[triangles_[i][0]]);
                    line_positions->push_back(state_.positions[triangles_[i][1]]);
                    line_positions->push_back(state_.positions[triangles_[i][2]]);
                    line_indices->push_back(0);
                    line_indices->push_back(1);
                    line_indices->push_back(1);
                    line_indices->push_back(2);
                    line_indices->push_back(2);
                    line_indices->push_back(0);
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
            else {
                prev_released = true;
            }
        }
    private:
        void InitIcosahedron(glm::vec3 center, float scale, glm::vec3 velocity) {
            float mass = 0.1f;
            bool vertices_fixed = false;
            bool center_fixed = true;
            // 12 vertices
            for (int i = 0; i < 12; i++) {
                positions_.push_back(icosa_vertices_[i] * scale + center);
                velocities_.push_back(velocity);
                system_.AddMass(mass, vertices_fixed);
            }
            // center
            positions_.push_back(icosa_vertices_[12] * scale + center);
            velocities_.push_back(velocity);
            system_.AddMass(mass, center_fixed);
            // 20 faces
            for (glm::vec3 face : icosa_faces_) {
                triangles_.push_back(face); // copy of icosa_faces_ for now
            }
        }

        std::vector<SceneNode*> sphere_node_ptrs_;
        //std::vector<SceneNode*> line_node_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> surface_line_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> radial_line_ptrs_;
        std::vector<glm::vec3> positions_;
        std::vector<glm::vec3> velocities_;
        std::vector<glm::vec3> triangles_;
        ParticleState state_;
        PendulumSystem system_;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
        float surface_l_ = 2.f;
        float surface_k_ = 50.f;
        float radial_l_ = 2.f;
        float radial_k_ = 50.f;
        float step_size_;

        // ICOSAHEDRON DATA
        const float t_ = (1.f + sqrt(5.f)) / 2.f;
        const std::vector<glm::vec3> icosa_vertices_{ // (x,y,z) coords for each vertex of nontransformed icosahedron
            glm::vec3(-1.f, t_, 0.f), // 12 vertices
            glm::vec3(1.f, t_, 0.f),
            glm::vec3(-1.f, -t_, 0.f),
            glm::vec3(1.f, -t_, 0.f),
            glm::vec3(0.f, -1.f, t_),
            glm::vec3(0.f, 1.f, t_),
            glm::vec3(0.f, -1.f, -t_),
            glm::vec3(0.f, 1.f, -t_),
            glm::vec3(t_, 0.f, -1),
            glm::vec3(t_, 0.f, 1.f),
            glm::vec3(-t_, 0.f, -1),
            glm::vec3(-t_, 0.f, 1.f),
            glm::vec3(0.f, 0.f, 0.f) // center
        };
        const std::vector<glm::vec3> icosa_faces_{ // (i,j,k) vertex indices for each face of icosahedron
            glm::vec3(0, 11, 5),
            glm::vec3(0, 5, 1),
            glm::vec3(0, 1, 7),
            glm::vec3(0, 7, 10),
            glm::vec3(0, 10, 11),
            glm::vec3(1, 5, 9),
            glm::vec3(5, 11, 4),
            glm::vec3(11, 10, 2),
            glm::vec3(10, 7, 6),
            glm::vec3(7, 1, 8),
            glm::vec3(3, 9, 4),
            glm::vec3(3, 4, 2),
            glm::vec3(3, 2, 6),
            glm::vec3(3, 6, 8),
            glm::vec3(3, 8, 9),
            glm::vec3(4, 9, 5),
            glm::vec3(2, 4, 11),
            glm::vec3(6, 2, 10),
            glm::vec3(8, 6, 7),
            glm::vec3(9, 8, 1),
        };
    };
} // namespace GLOO

#endif

