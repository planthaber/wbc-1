#include "VelocitySceneQuadraticCost.hpp"
#include <tools/SVD.hpp>
#include <base/JointLimits.hpp>
#include <base-logging/Logging.hpp>

namespace wbc{

VelocitySceneQuadraticCost::VelocitySceneQuadraticCost(RobotModelPtr robot_model, QPSolverPtr solver) :
    VelocityScene(robot_model, solver),
    min_eval_damping_thresh(0),
    damping_factor(1){

}

VelocitySceneQuadraticCost::~VelocitySceneQuadraticCost(){
}

const HierarchicalQP& VelocitySceneQuadraticCost::update(){

    if(!configured)
        throw std::runtime_error("VelocitySceneQuadraticCost has not been configured!. PLease call configure() before calling update() for the first time!");

    if(constraints.size() != 1){
        LOG_ERROR("Number of priorities in VelocitySceneQuadraticCost should be 1, but is %i", constraints.size());
        throw std::runtime_error("Invalid constraint configuration");
    }

    VelocityScene::update();

    int nj = robot_model->noOfJoints();
    std::vector<std::string> contact_points = robot_model->getContactPoints();
    uint ncp = contact_points.size();
    uint prio = 0;
    constraints_prio[prio].nc = ncp*6;

    constraints_prio[prio].H = constraints_prio[prio].A.transpose()*constraints_prio[prio].A;
    constraints_prio[prio].g = -(constraints_prio[prio].A.transpose()*constraints_prio[prio].lower_y).transpose();
    constraints_prio[prio].lower_y.resize(0);
    constraints_prio[prio].upper_y.resize(0);
    constraints_prio[prio].lower_x.setConstant(nj, -std::numeric_limits<double>::max());
    constraints_prio[prio].upper_x.setConstant(nj, std::numeric_limits<double>::max());
    for(auto n : robot_model->actuatedJointNames()){
        size_t idx = robot_model->jointIndex(n);
        const base::JointLimitRange &range = robot_model->jointLimits().getElementByName(n);
        constraints_prio[prio].lower_x(idx) = -10000;//range.min.speed;
        constraints_prio[prio].upper_x(idx) = 10000;//range.max.speed;//
    }

    // Compute variable damping depending on smallest Eigenvalue
//        if(min_eval_damping_thresh > 0){
//            int nc = n_constraint_variables_per_prio[prio];
//            s_vals.resize(nj);
//            tmp.resize(nj);
//            sing_vect_r.resize(nj, nj);
//            U.resize(nj, nj);
//            svd_eigen_decomposition(constraints_prio[prio].H, U, s_vals, sing_vect_r, tmp);

//            double s_min = s_vals.block(0,0,std::min(nj, nc),1).minCoeff();
//            if(s_min <= min_eval_damping_thresh)
//                damping_factor = s_min*(1-min_eval_damping_thresh)/(min_eval_damping_thresh) + 1e-6;
//            else
//                damping_factor = 1;
//            constraints_prio[prio].g*=damping_factor;
//        }

    // For all contacts: Js*qd = 0 (Rigid Contacts, contact points do not move!)
    constraints_prio[prio].A.resize(ncp*6,nj);
    constraints_prio[prio].A.setZero();
    for(int i = 0; i < contact_points.size(); i++)
        constraints_prio[prio].A.block(i*6, 0, 6, nj) = robot_model->spaceJacobian(robot_model->baseFrame(), contact_points[i]);
    constraints_prio[prio].lower_y.resize(ncp*6);
    constraints_prio[prio].upper_y.resize(ncp*6);
    constraints_prio[prio].lower_y.setZero();
    constraints_prio[prio].upper_y.setZero();

    return constraints_prio;
}

} // namespace wbc
