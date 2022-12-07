#ifndef TRAPEZOIDAL_INTEGRATOR_H_
#define TRAPEZOIDAL_INTEGRATOR_H_

#include "IntegratorBase.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class TrapezoidalIntegrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    ParticleState f_0 = system.ComputeTimeDerivative(state, start_time);
    ParticleState f_1 = system.ComputeTimeDerivative(state+dt*f_0, start_time+dt);
    TState new_state = state + dt/2 * (f_0 + f_1);
    return new_state;
  }
};
}  // namespace GLOO

#endif
