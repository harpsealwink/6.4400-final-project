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
#include "GroundNode.hpp"
#include <cstdlib>
#include <glm/gtx/string_cast.hpp>


namespace GLOO {
    class BallNode : public SceneNode {
    public:
        BallNode(IntegratorType integrator_type, float integration_step) {
            // UI
            drop_ball_ = false;

            integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type);
            step_size_ = integration_step;

            // initialize icosphere
            InitIcosahedron();
            SubdivideToIcosphere();
            state_ = { positions_, velocities_ };

            // render vertices
            if (display_vertices_) {
                for (size_t i = 0; i < positions_.size(); i++) {
                    auto sphere_node = make_unique<SceneNode>();
                    sphere_node->CreateComponent<MaterialComponent>(red_material_);
                    sphere_node->CreateComponent<ShadingComponent>(shader_);
                    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
                    sphere_node_ptrs_.push_back(sphere_node.get());
                    AddChild(std::move(sphere_node));
                }
            }

            // add radial springs
            for (size_t i = 1; i < positions_.size(); i++) {
                system_.AddSpring(0, i, radial_l_, radial_k_);
                if (display_radii_) {
                    auto line_node = make_unique<SceneNode>();
                    line_node->CreateComponent<MaterialComponent>(green_material_);
                    line_node->CreateComponent<ShadingComponent>(line_shader_);

                    auto positions = make_unique<PositionArray>();
                    auto indices = make_unique<IndexArray>();
                    auto line = std::make_shared<VertexObject>();
                    positions->push_back(positions_[0]); // center node
                    positions->push_back(positions_[i]);
                    indices->push_back(0);
                    indices->push_back(1);
                    line->UpdatePositions(std::move(positions));
                    line->UpdateIndices(std::move(indices));

                    auto& rc_curve = line_node->CreateComponent<RenderingComponent>(line);
                    rc_curve.SetDrawMode(DrawMode::Lines);
                    radial_line_ptrs_.push_back(line);
                    AddChild(std::move(line_node));
                }
            }

            // add chordal springs
            for (size_t i = 1; i < positions_.size(); i++) {
                for (size_t j = 1; j < i; j++) {
                    system_.AddSpring(i, j, glm::length(positions_[i] - positions_[j]), chordal_k_);
                    if (display_chords_) {
                        auto line_node = make_unique<SceneNode>();
                        line_node->CreateComponent<MaterialComponent>(green_material_);
                        line_node->CreateComponent<ShadingComponent>(line_shader_);

                        auto positions = make_unique<PositionArray>();
                        auto indices = make_unique<IndexArray>();
                        auto line = std::make_shared<VertexObject>();
                        positions->push_back(positions_[i]); // center node
                        positions->push_back(positions_[j]);
                        indices->push_back(0);
                        indices->push_back(1);
                        line->UpdatePositions(std::move(positions));
                        line->UpdateIndices(std::move(indices));

                        auto& rc_curve = line_node->CreateComponent<RenderingComponent>(line);
                        rc_curve.SetDrawMode(DrawMode::Lines);
                        chordal_line_ptrs_.push_back(line);
                        AddChild(std::move(line_node));
                    }
                }
            }

            // add surface springs
            for (size_t i = 0; i < triangles_.size(); i++) {
                int i0 = triangles_[i][0];
                int i1 = triangles_[i][1];
                int i2 = triangles_[i][2];
                glm::vec3 v0 = positions_[i0];
                glm::vec3 v1 = positions_[i1];
                glm::vec3 v2 = positions_[i2];
                system_.AddSpring(i0, i1, glm::length(v1 - v0), surface_k_);
                system_.AddSpring(i1, i2, glm::length(v2 - v1), surface_k_);
                system_.AddSpring(i2, i0, glm::length(v0 - v2), surface_k_);

                if (display_mesh_) {
                    auto line_node = make_unique<SceneNode>();
                    line_node->CreateComponent<MaterialComponent>(green_material_);
                    line_node->CreateComponent<ShadingComponent>(line_shader_);

                    auto positions = make_unique<PositionArray>();
                    auto indices = make_unique<IndexArray>();
                    auto line = std::make_shared<VertexObject>();
                    positions->push_back(v0);
                    positions->push_back(v1);
                    positions->push_back(v2);
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
                }
            }

            // render surface
            ComputeNormals();
            if (display_surface_) {
                auto surface_node = make_unique<SceneNode>();
                surface_node->CreateComponent<ShadingComponent>(shader_);
                surface_node->CreateComponent<MaterialComponent>(white_material_);
                surface_node->CreateComponent<RenderingComponent>(normal_mesh_);
                AddChild(std::move(surface_node));
            }

            // make other objects (for collision purposes)
            auto ground_node = make_unique<GroundNode>();
            ground_ptr_ = ground_node.get();
        };

        void Reset() {
            // reset state by clearing elements
            positions_.clear();
            velocities_.clear();
            triangles_.clear();

            InitIcosahedron();
            SubdivideToIcosphere();
            state_ = { positions_, velocities_ };
        }


        void Update(double delta_time) {
            static bool prev_released_d = true;
            if (InputManager::GetInstance().IsKeyPressed('D')) {
                if (prev_released_d) {
                    drop_ball_ = true;
                }
                prev_released_d = false;
            }
            else {
                prev_released_d = true;
            }

            double start_time = 0.0;
            while (start_time < delta_time) {
                state_ = integrator_->Integrate(system_, state_, start_time, fmin(step_size_, delta_time)); // step sizes cannot be greater than time

                if (!drop_ball_) {
                    state_.velocities = velocities_;
                }

                // update vertices
                for (size_t i = 0; i < state_.positions.size(); i++) {
                    // float lower = 0.0;
                    float eps = 0.01;
                    if (ground_ptr_->InBounds(state_.positions[i], eps)) {
                        state_.velocities[i] = glm::vec3(0.f, 1.f, 0.f);
                    }
                    if (display_vertices_) {
                        sphere_node_ptrs_[i]->GetTransform().SetPosition(state_.positions[i]);
                    }
                }

                // update radial springs
                if (display_radii_) {
                    for (size_t i = 1; i < state_.positions.size(); i++) {
                        auto line = radial_line_ptrs_[i - 1];
                        auto line_positions = make_unique<PositionArray>();
                        auto line_indices = make_unique<IndexArray>();
                        line_positions->push_back(state_.positions[0]);
                        line_positions->push_back(state_.positions[i]);
                        line->UpdatePositions(std::move(line_positions));
                    }
                }

                // update chordal springs
                if (display_chords_) {
                    for (size_t i = 1; i < state_.positions.size(); i++) {
                        for (size_t j = i; j < i; j++) {
                            auto line = chordal_line_ptrs_[i - 1];
                            auto line_positions = make_unique<PositionArray>();
                            auto line_indices = make_unique<IndexArray>();
                            line_positions->push_back(state_.positions[0]);
                            line_positions->push_back(state_.positions[i]);
                            line->UpdatePositions(std::move(line_positions));
                        }
                    }
                }


                // update surface springs
                if (display_mesh_) {
                    for (size_t i = 0; i < triangles_.size(); i++) {
                        auto line = surface_line_ptrs_[i];
                        auto line_positions = make_unique<PositionArray>();
                        auto line_indices = make_unique<IndexArray>();
                        line_positions->push_back(state_.positions[triangles_[i][0]]);
                        line_positions->push_back(state_.positions[triangles_[i][1]]);
                        line_positions->push_back(state_.positions[triangles_[i][2]]);
                        line->UpdatePositions(std::move(line_positions));
                    }
                }

                // update normals
                ComputeNormals();

                start_time += step_size_;
            }

            static bool prev_released = true;
            if (InputManager::GetInstance().IsKeyPressed('R')) {
                if (prev_released) {
                    drop_ball_ = false;
                    state_ = { positions_, velocities_ };
                }
                prev_released = false;
            }
            else {
                prev_released = true;
            }
        }


        // GUI Functions
        void LinkControl(float*& height, float*& x, float*& z) {
            linked_height_ = height;
            linked_x_ = x;
            linked_z_ = z;
        }
        void OnParamsChanged() {
            start_center_ = glm::vec3(*linked_x_, *linked_height_, *linked_z_);
            Reset();
        }


    private:
        void InitIcosahedron() {
            // center
            AddVertex(icosa_vertices_[0] * scale_ + start_center_, center_mass_, center_fixed_);

            // 12 vertices
            for (int i = 1; i <= 12; i++) {
                AddVertex(icosa_vertices_[i] * scale_ + start_center_, vertex_mass_, vertex_fixed_);
            }

            // 20 faces
            for (glm::vec3 face : icosa_faces_) {
                triangles_.push_back(face + glm::vec3(1, 1, 1)); // adjust indices by one since center is at index 0
            }
        }
        void SubdivideToIcosphere() {
            // http://www.songho.ca/opengl/gl_sphere.html
            //         v0       
            //        / \       
            //    v3 *---* v5
            //      / \ / \     
            //    v1---*---v2   
            //         v4     
            for (int n = 0; n < subdivisions_ - surface_layers_ + 1; n++) { // final icosphere only includes last layer of this loop
                std::vector<glm::vec3> temp_triangles;
                for (glm::vec3 triangle : triangles_) {
                    // original vertices
                    int i0 = triangle[0];
                    int i1 = triangle[1];
                    int i2 = triangle[2];

                    // new vertices
                    int i3 = AddMidpoint(i0, i1, vertex_mass_, vertex_fixed_);
                    int i4 = AddMidpoint(i1, i2, vertex_mass_, vertex_fixed_);
                    int i5 = AddMidpoint(i2, i0, vertex_mass_, vertex_fixed_);

                    // new faces
                    temp_triangles.push_back(glm::vec3(i0, i3, i5));
                    temp_triangles.push_back(glm::vec3(i3, i1, i4));
                    temp_triangles.push_back(glm::vec3(i5, i4, i2));
                    temp_triangles.push_back(glm::vec3(i3, i4, i5));
                }
                midpt_cache_.clear();
                triangles_ = temp_triangles;
            }
            std::vector<glm::vec3> new_triangles = triangles_; // if surface_layers == subdivisions_ + 1, then this is original triangles_ (otherwise, comes from previous loop)
            for (int n = subdivisions_ - surface_layers_ + 1; n < subdivisions_; n++) {
                for (glm::vec3 triangle : triangles_) {
                    // original vertices
                    int i0 = triangle[0];
                    int i1 = triangle[1];
                    int i2 = triangle[2];

                    // new vertices
                    int i3 = AddMidpoint(i0, i1, vertex_mass_, vertex_fixed_);
                    int i4 = AddMidpoint(i1, i2, vertex_mass_, vertex_fixed_);
                    int i5 = AddMidpoint(i2, i0, vertex_mass_, vertex_fixed_);

                    // new faces
                    new_triangles.push_back(glm::vec3(i0, i3, i5));
                    new_triangles.push_back(glm::vec3(i3, i1, i4));
                    new_triangles.push_back(glm::vec3(i5, i4, i2));
                    new_triangles.push_back(glm::vec3(i3, i4, i5));
                }
                midpt_cache_.clear();
                triangles_ = new_triangles;
            }
        }
        void AddVertex(glm::vec3 position, float mass, bool fixed) {
            positions_.push_back(position);
            velocities_.push_back(start_velocity_);
            system_.AddMass(mass, fixed);
        }
        int AddMidpoint(int i0, int i1, float mass, bool fixed) {
            int i2 = GetMidpointIndex(i0, i1);
            if (i2 == positions_.size()) {
                glm::vec3 v0 = positions_[i0] - positions_[0];
                glm::vec3 v1 = positions_[i1] - positions_[0];
                glm::vec3 v2 = glm::normalize(v0 + v1) * (glm::length(v0) + glm::length(v1)) / 2.f;
                AddVertex(v2 + positions_[0], mass, fixed);
            }
            return i2;
        }
        int GetMidpointIndex(int i0, int i1) { // indices of endpts
            int key = (i0 * i1 << 12) + (i0 + i1); // "hash" of unordered pair (i0, i1)
            auto search = midpt_cache_.find(key);
            if (search == midpt_cache_.end()) { // midpoint is a new vertex
                midpt_cache_.insert({ key, positions_.size() });
                return positions_.size();
            }
            else {
                return search->second; // midpoint is already a vertex
            }
        }
        void ComputeNormals() { // add surface normals to sphere (simultaneously calculate areas and volume)
            auto normal_positions = make_unique<PositionArray>();
            auto normal_indicies = make_unique<IndexArray>();
            auto normals = make_unique<NormalArray>();
            std::vector<glm::vec3> normal_sums;
            float volume = 0.f;
            glm::vec3 p = state_.positions[0]; // anchor point to calculate volume of each tetrahedron

            for (int i = 0; i < state_.positions.size(); i++) {
                normal_positions->push_back(state_.positions[i]); // load in all positions
                normal_sums.push_back(glm::vec3(0.f)); // initialize normals array
            }
            for (glm::vec3 triangle : triangles_) {
                // load in all triangle indicies
                normal_indicies->push_back(triangle[0]);
                normal_indicies->push_back(triangle[1]);
                normal_indicies->push_back(triangle[2]);

                // add the normals of incident faces to each vertex normal
                int idx1 = triangle[0];
                int idx2 = triangle[1];
                int idx3 = triangle[2];
                glm::vec3 v1 = state_.positions[idx2] - state_.positions[idx1];
                glm::vec3 v2 = state_.positions[idx3] - state_.positions[idx1];
                glm::vec3 normal = glm::cross(v1, v2);
                normal_sums[idx1] += normal;
                normal_sums[idx2] += normal;
                normal_sums[idx3] += normal;

                // calculate signed volume of tetrahedron with vertex "p" and opposite face "triangle"
                glm::vec3 w1 = state_.positions[idx1] - p;
                glm::vec3 w2 = state_.positions[idx2] - p;
                glm::vec3 w3 = state_.positions[idx3] - p;
                volume += glm::dot(w1, glm::cross(w2, w3)) / 6.f; // triple product for signed volume
            }
            for (int i = 0; i < normal_sums.size(); i++) {
                normals->push_back(glm::normalize(normal_sums[i])); // normalize the sum of normals for vertex
            }

            normal_mesh_->UpdatePositions(std::move(normal_positions));
            normal_mesh_->UpdateIndices(std::move(normal_indicies));
            normal_mesh_->UpdateNormals(std::move(normals));
            system_.SetNormals(normal_sums);
            system_.SetVolume(fabs(volume));
        }

        bool OutOfBounds(glm::vec3 position, float lower, float eps) {
            //if ((position.x > lower_left.x + eps) && (position.x < upper_right.x - eps)    // within left-right bounds
                //&& (position.y > lower_left.y + eps) && (position.y < upper_right.y - eps)) { // within lower-upper bounds
            if (position.y > lower + eps) {
                return false;
            }
            return true;
        }

        // SCENENODE PROPERTIES
        std::shared_ptr<Material> red_material_ = std::make_shared<Material>(
            glm::vec3(1.f, 0.f, 0.f),
            glm::vec3(1.f, 0.f, 0.f),
            glm::vec3(1.f, 0.f, 0.f), 20.0f);
        std::shared_ptr<Material> green_material_ = std::make_shared<Material>(
            glm::vec3(0.f, 1.f, 0.f),
            glm::vec3(0.f, 1.f, 0.f),
            glm::vec3(0.f, 1.f, 0.f), 20.0f);
        std::shared_ptr<Material> blue_material_ = std::make_shared<Material>(
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 0.f, 1.f), 20.0f);
        std::shared_ptr<Material> white_material_ = std::make_shared<Material>(
            glm::vec3(1.f, 0.8f, 1.f),
            glm::vec3(1.f, 0.8f, 1.f),
            glm::vec3(0.2f, 0.2f, 0.2f), 20.0f);
        std::shared_ptr<SimpleShader> line_shader_ = std::make_shared<SimpleShader>();
        std::shared_ptr<PhongShader> shader_ = std::make_shared<PhongShader>();
        std::shared_ptr<VertexObject> sphere_mesh_ = PrimitiveFactory::CreateSphere(0.03f, 25, 25);
        std::shared_ptr<VertexObject> normal_mesh_ = std::make_shared<VertexObject>();

        // SCENENODE POINTERS
        std::vector<SceneNode*> sphere_node_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> surface_line_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> chordal_line_ptrs_;
        std::vector<std::shared_ptr<VertexObject>> radial_line_ptrs_;
        //SceneNode* mesh_node_;

        // SIMULATION INFO
        std::vector<glm::vec3> positions_;
        std::vector<glm::vec3> velocities_;
        std::vector<glm::vec3> triangles_;
        ParticleState state_;
        PendulumSystem system_;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
        float step_size_;

        // DISPLAY TOGGLES 
        bool display_vertices_ = false;
        bool display_chords_ = true;
        bool display_radii_ = false;
        bool display_mesh_ = false;
        bool display_surface_ = false;

        // ICOSPHERE PARAMS
        glm::vec3 start_center_ = glm::vec3(0.f, 1.f, 0.f);
        glm::vec3 start_velocity_ = glm::vec3(0.f, 0.f, 0.f);
        bool center_fixed_ = false;
        bool vertex_fixed_ = false;
        const float scale_ = 0.2;
        const int subdivisions_ = 3;
        const int surface_layers_ = 1; // must have 1 <= surface_layers_ <= subdivisions_ + 1
        const float center_mass_ = 0.01f;
        const float vertex_mass_ = 0.0001;
        const float surface_k_ = 30.f;
        const float chordal_k_ = 10.0f;
        const float radial_k_ = 0.0f;
        const float radial_l_ = 1.90211 * scale_; // circumradius
        std::unordered_map<int, int> midpt_cache_;

        // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
        // ICOSAHEDRON DATA (edge length 2) 
        const float t_ = (1.f + sqrt(5.f)) / 2.f;
        const std::vector<glm::vec3> icosa_vertices_{ // (x,y,z) coords for each vertex of nontransformed icosahedron
            glm::vec3(0.f, 0.f, 0.f), // center
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
        };
        const std::vector<glm::vec3> icosa_faces_{ // (i,j,k) vertex indices for each face of icosahedron
            glm::vec3(0, 11, 5),                   // indexed by 0 (notice icosa_vertices_ are indexed by 1
            glm::vec3(0, 5, 1),                    // since the center is at the 0th index)
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

        // UI Controls
        bool drop_ball_;
        float* linked_height_;
        float* linked_x_;
        float* linked_z_;

        // other objects
        GroundNode* ground_ptr_;
    };
} // namespace GLOO

#endif

