#ifndef CARTESIANCONSTRAINT_HPP
#define CARTESIANCONSTRAINT_HPP

#include "Constraint.hpp"

namespace base{ namespace samples { class CartesianState; } }

namespace wbc {

/**
 * @brief Abstract interface for a constraint in Cartesian space
 */
class CartesianConstraint : public Constraint{
public:
    CartesianConstraint(const ConstraintConfig& _config, uint n_robot_joints);
    virtual ~CartesianConstraint();

    /**
     * @brief Update the Cartesian reference input for this constraint.
     */
    virtual void setReference(const base::samples::CartesianState& ref);
};

} //namespace wbc

#endif
