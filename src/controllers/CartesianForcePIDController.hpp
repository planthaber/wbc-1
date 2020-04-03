#ifndef WBC_CONTROLLERS_CARTESIANFORCEPIDCONTROLLER_HPP
#define WBC_CONTROLLERS_CARTESIANFORCEPIDCONTROLLER_HPP

#include "PIDController.hpp"
#include <base/samples/Wrench.hpp>
#include <ctrl_types/RigidBodyStateSE3.hpp>

namespace ctrl_lib {

/**
 * @brief The JointPosPDController class implements the PosPDController in joint space
 */
class CartesianForcePIDController : public PIDController{
protected:
    base::samples::RigidBodyStateSE3 control_output_wrench;

    const base::VectorXd wrenchToRaw(const base::samples::Wrench& wrench, base::VectorXd& raw);

public:
    CartesianForcePIDController();

    /** Convert typed to raw input data and call PIDController::update()*/
    const base::samples::RigidBodyStateSE3& update(const base::samples::Wrench& setpoint, const base::samples::Wrench& feedback, const double dt);
};

}

#endif
