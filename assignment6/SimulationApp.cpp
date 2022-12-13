#include "SimulationApp.hpp"

#include "glm/gtx/string_cast.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "BallNode.hpp"
#include "GroundNode.hpp"


namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             IntegratorType integrator_type,
                             float integration_step)
    : Application(app_name, window_size),
      integrator_type_(integrator_type),
      integration_step_(integration_step) {
}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 5.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.2f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(1.f, 1.f, 1.f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(3.0f, 5.0f, 0.f));
  root.AddChild(std::move(point_light_node));

  auto ball_node = make_unique<BallNode>(integrator_type_, integration_step_);
  ball_node_ptr_ = ball_node.get();
  root.AddChild(std::move(ball_node));

  float *height = &ball_height_;
  float *x = &ball_x_;
  float *z = &ball_z_;
  ball_node_ptr_->LinkControl(height, x, z);
  ball_node_ptr_->OnParamsChanged();

  auto ground_node = make_unique<GroundNode>();
  root.AddChild(std::move(ground_node));
}

void SimulationApp::DrawGUI() {
  bool modified = false;
  ImGui::Begin("Controls");
  ImGui::Text("Ball Parameters");
  ImGui::PushID(0);
  modified |= ImGui::SliderFloat("y (height)", &ball_height_, 0.0, 6);
  ImGui::PopID();
  ImGui::PushID(1);
  modified |= ImGui::SliderFloat("x", &ball_x_, -10, 10);
  ImGui::PopID();
  ImGui::PushID(2);
  modified |= ImGui::SliderFloat("z", &ball_z_, -10, 10);
  ImGui::PopID();
  ImGui::End();

  if (modified) {
    ball_node_ptr_->OnParamsChanged();
  }
}
}  // namespace GLOO
