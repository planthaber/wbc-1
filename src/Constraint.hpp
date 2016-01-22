#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include <base/Eigen.hpp>
#include "ConstraintConfig.hpp"
#include <base/time.h>
#include <base/samples/RigidBodyState.hpp>
#include <base/samples/Joints.hpp>
#include <base/logging.h>

namespace wbc{

/**
 * @brief Class to carry constraint specific information
 */
class Constraint{
public:

    Constraint(){
    }
    Constraint(const ConstraintConfig& _config, const uint n_robot_joints)
    {

        config = _config;

        if(config.type == jnt)
            no_variables = _config.joint_names.size();
        else
            no_variables = 6;

        if(config.weights.size() != no_variables){
            LOG_ERROR("Constraint '%s' has %i variables, but its weights vector has size %i",
                      config.name.c_str(), no_variables, config.weights.size());
            throw std::invalid_argument("Invalid WBC config");
        }
        if(config.activation < 0 || config.activation > 1){
            LOG_ERROR("Activation of constraint '%s' is %f. It has to be be between 0 and 1",
                      config.name.c_str(),config.activation);
            throw std::invalid_argument("Invalid WBC config");
        }

        for(uint i = 0; i < config.weights.size(); i++)
        {
            if(config.weights[i] < 0){
                LOG_ERROR("Weight no %i of constraint '%s' is %f. It has to be >= 0",
                          i, config.name.c_str(), config.weights[i]);
                throw std::invalid_argument("Invalid WBC config");

            }
        }

        y_ref.resize(no_variables);
        y_ref_root.resize(no_variables);
        A.resize(no_variables, n_robot_joints);
        weights.resize(no_variables);
        y_solution.resize(no_variables);
        y.resize(no_variables);
        error_y_solution.resize(no_variables);
        error_y.resize(no_variables);
        sqrt_sum_error_y_solution.resize(no_variables);
        sqrt_sum_error_y.resize(no_variables);
        rms_error_y_solution.resize(no_variables);
        rms_error_y.resize(no_variables);
        n_samples.resize(no_variables);
        mask.resize(no_variables);

        reset();
    }

    base::Time time;

    /** Configuration of this constraint. */
    ConstraintConfig config;

    /** Reference value for this constraint. Either joint space values or a Cartesian Twist defined in ref_frame coordinates*/
    base::VectorXd y_ref;

    /** Reference value for this constraint. Either joint space values or a Cartesian Twist defined in root coordinates*/
    base::VectorXd y_ref_root;

    /** Constraint weights, a 0 means that the reference of the corresponding constraint variable will be ignored while computing the solution*/
    base::VectorXd weights;

    /** Between 0 .. 1. Will be multiplied with the constraint weights. Can be used to (smoothly) switch on/off the constraints */
    double activation;

    /** Can be 0 or 1. Will be multiplied with the constraint weights. If no new reference values arrives for a certain time, the constraint times out*/
    int constraint_timed_out;

    /** Constraint matrix */
    base::MatrixXd A;

    /** Number of constraint variables */
    uint no_variables;

    /** last time a new reference sample arrived*/
    base::Time last_ref_input;

    /** Solution as computed by the solver for this constraint. For Cartesian constraints, this will be back transformed to
     *  Cartesian space and defined in root coordinates*/
    base::VectorXd y_solution;

    /** Actual constraint as executed on the robot. For Cartesian constraints, this will be back transformed to
     *  Cartesian space and defined in root coordinates*/
    base::VectorXd y;

    /** | y_ref_root - y_solution |: The current error generated by the solver (E.g. through Singularities, ...)*/
    base::VectorXd error_y_solution;

    /** | y_ref_root - y |: The current error generated by the solver AND the robot (e.g. through infeasible trajectories, bad joint controllers, ...)*/
    base::VectorXd error_y;

    /** Sum of the mean square error*/
    base::VectorXd sqrt_sum_error_y_solution;

    /** Sum of the mean square error*/
    base::VectorXd sqrt_sum_error_y;

    /** Root mean square error over all relevant samples. Relevant means here, that only samples will be counted where activation and weights are non-zero*/
    base::VectorXd rms_error_y_solution;

    /** Root mean square error over all relevant samples. Relevant means here, that only samples will be counted where activation and weights are non-zero*/
    base::VectorXd rms_error_y;

    /** Counter for rms computation*/
    base::VectorXd n_samples;

    /** Helper for counting only relevant samples. Relevant means here, that only samples will be counted where activation and weights are non-zero*/
    base::VectorXd mask;

    void setReference(const base::samples::RigidBodyState &ref);
    void setReference(const base::samples::Joints& ref);
    void updateTime();
    void validate();
    void reset();
    void resetEvaluation();
    void evaluate(const base::VectorXd& solver_output, const base::VectorXd& actual_robot_vel);

    /** Compute element wise absolute value of vector*/
    void VectAbs(base::VectorXd& in);
};
typedef std::vector<Constraint> ConstraintsPerPrio;

}
#endif
