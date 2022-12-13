#ifndef GROUND_NODE_H_
#define GROUND_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include <glm/gtx/string_cast.hpp>


namespace GLOO {
    class GroundNode : public SceneNode {
    public:
        GroundNode() {
            auto normal_positions = make_unique<PositionArray>();
            auto normal_indicies = make_unique<IndexArray>();
            auto normals = make_unique<NormalArray>();

            positions_.push_back(glm::vec3(left_edge_, height_, back_edge_));
            positions_.push_back(glm::vec3(left_edge_, height_, front_edge_));
            positions_.push_back(glm::vec3(right_edge_, height_, back_edge_));
            positions_.push_back(glm::vec3(right_edge_, height_, front_edge_));
            for (glm::vec3 position : positions_) {
                normal_positions->push_back(position);
            }

            normal_indicies->push_back(0);
            normal_indicies->push_back(1);
            normal_indicies->push_back(2);
            normal_indicies->push_back(2);
            normal_indicies->push_back(1);
            normal_indicies->push_back(3);

            normals->push_back(glm::cross(positions_[1]-positions_[0], positions_[2]-positions_[0]));
            normals->push_back(glm::cross(positions_[1]-positions_[2], positions_[3]-positions_[2]));

            normal_mesh_->UpdatePositions(std::move(normal_positions));
            normal_mesh_->UpdateIndices(std::move(normal_indicies));
            normal_mesh_->UpdateNormals(std::move(normals));

            auto surface_node = make_unique<SceneNode>();
            surface_node->CreateComponent<ShadingComponent>(shader_);
            surface_node->CreateComponent<MaterialComponent>(material_);
            surface_node->CreateComponent<RenderingComponent>(normal_mesh_);
            AddChild(std::move(surface_node));
        }

        bool InBounds(glm::vec3 position) {
            if (position.y < height_) {
                if (position.x > left_edge_ && position.x < right_edge_) {
                    if (position.z > back_edge_ && position.z < front_edge_) {
                        return true;
                    }
                }
            }
            return false;
        }


    private:
        float height_ = 0.0; // y
        float left_edge_ = -5.0; // -x
        float right_edge_ = 5.0; // +x
        float back_edge_ = -5.0; // -z
        float front_edge_ = 5.0; // +z
        std::vector<glm::vec3> positions_;

        std::shared_ptr<VertexObject> normal_mesh_ = std::make_shared<VertexObject>();
        std::shared_ptr<PhongShader> shader_ = std::make_shared<PhongShader>();
        std::shared_ptr<Material> material_ = std::make_shared<Material>(
            glm::vec3(0.2f, 0.2f, 1.f),
            glm::vec3(0.2f, 0.2f, 1.f),
            glm::vec3(0.2f, 0.2f, 0.2f), 20.0f);
    };
} // namespace GLOO

#endif

