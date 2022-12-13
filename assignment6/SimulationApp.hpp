#ifndef SIMULATION_APP_H_
#define SIMULATION_APP_H_

#include "gloo/Application.hpp"

#include "IntegratorType.hpp"
#include "BallNode.hpp"

namespace GLOO {
class SimulationApp : public Application {
  public:
    SimulationApp(const std::string& app_name,
                  glm::ivec2 window_size,
                  IntegratorType integrator_type,
                  float integration_step);
    void SetupScene() override;

  protected:
    void DrawGUI() override;

  private:
    IntegratorType integrator_type_;
    float integration_step_;

    // GUI stuff
    BallNode* ball_node_ptr_;
    float ball_height_ = 1.f;
    float ball_x_ = 0.f;
    float ball_z_ = 0.f;
};
}  // namespace GLOO

#endif
