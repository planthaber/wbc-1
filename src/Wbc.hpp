#ifndef WBC_HPP
#define WBC_HPP

#include "WbcTypes.hpp"
#include <kdl/tree.hpp>
#include <base/commands/joints.h>
#include "TaskFrame.hpp"
#include <base/Eigen.hpp>
#include "RobotModel.hpp"
#include "HierarchicalWDLSSolver.hpp"

namespace wbc{

/**
 * Vector of vectors of task variables.
 * Each priority level may have several sub tasks and each task may have multiple task variables
 */
typedef std::vector< std::vector<base::VectorXd> > WbcInput;

/**
 * Vector of vectors of Sub Task Configurations.
 * Each priority level may have several sub tasks, each of which is described by a SubTaskConfig.
 */
typedef std::vector< std::vector<SubTaskConfig> > WbcConfig;

/**
 * @brief helper class to carry sub task specific information
 */
class SubTask{
public:
    SubTask(const SubTaskConfig& config, const uint no_robot_joints){
        config_ = config;
        uint no_task_vars;
        if(config.type == cartesian)
            no_task_vars = 6;
        else
            no_task_vars = config.joints.size();

        y_des_.resize(no_task_vars);

        task_weights_.resize(no_task_vars);
        task_weights_.setConstant(1);

        task_jac_ = KDL::Jacobian(no_task_vars);
        task_jac_.data.setZero();

        H_.resize(no_task_vars,6);
        H_.setZero();

        Uf_.resize(6, no_task_vars);
        Uf_.setIdentity();

        Vf_.resize(no_task_vars, no_task_vars);
        Vf_.setIdentity();

        Sf_.resize(no_task_vars);
        Sf_.setZero();

        A_.resize(no_task_vars, no_robot_joints);
        A_.setZero();
    }

    ~SubTask(){}

    SubTaskConfig config_;

    Eigen::VectorXd y_des_;
    Eigen::VectorXd task_weights_;
    KDL::Jacobian task_jac_;
    KDL::Frame pose_;
    Eigen::MatrixXd A_;

    //Helpers for inversion of the task Jacobian
    Eigen::MatrixXd Uf_, Vf_;
    Eigen::VectorXd Sf_;
    Eigen::MatrixXd H_;
};

class Wbc{
public:
    HierarchicalWDLSSolver solver_;
    RobotModel *robot_; /** Instance of the KDL robot model. Created at construction time of wbc. Contains task frames, Jacobians, etc...*/

    std::vector< std::vector<SubTask*> > sub_task_vector_; /** Vector containing all sub tasks (ordered by priority, highest priority first) */
    Eigen::VectorXd solver_output_; /** Control solution. Size: No of joints in kdl tree. Order of joints will be same as in status vector given in solve() */

    std::vector<Eigen::MatrixXd> A_; /** Vector of task matrices per priority. These define, together with y, for each priority the linear equation system that has to be solved */
    std::vector<Eigen::VectorXd> y_ref_; /** Vector of desired task variables per priority */
    std::vector<Eigen::VectorXd> y_; /** Vector of actual task variables according to the computed control solution*/
    std::vector<Eigen::VectorXd> Wy_; /** Vector of task weights per priority */

    bool configured_;

    //Helpers
    Eigen::VectorXd temp_;

    void updateSubTask(SubTask* sub_task);

    /**
     * @brief Create Robot Model and Solver
     * @param tree Kinematic tree for the robot model
     * @param type Which wbc to use?
     */
    Wbc(KDL::Tree tree);
    ~Wbc();

    /**
     * @brief Configure Has to be called before calling solve for the first time.
     * @param config Sub task configuration vector.
     * @return True in case of success, false else
     */
    bool configure(const WbcConfig &config);

    /**
     * @brief solve
     * @param task_ref
     * @param task_weights
     * @param joint_weights
     * @param robot_status
     * @param solver_output
     */
    void solve(const WbcInput& task_ref,
               const WbcInput& task_weights,
               const base::VectorXd joint_weights,
               const base::samples::Joints &robot_status,
               base::samples::Joints &solver_output);

};
}
#endif

