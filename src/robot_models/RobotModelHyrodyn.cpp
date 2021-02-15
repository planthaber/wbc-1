#include "RobotModelHyrodyn.hpp"
#include <base-logging/Logging.hpp>
#include <urdf_parser/urdf_parser.h>

namespace wbc{

RobotModelHyrodyn::RobotModelHyrodyn(){
}

RobotModelHyrodyn::~RobotModelHyrodyn(){
}

bool RobotModelHyrodyn::configure(const RobotModelConfig& cfg){

    if(!RobotModel::configure(cfg))
        return false;

    TiXmlDocument *doc = urdf::exportURDF(robot_urdf);
    std::string robot_urdf_file = "/tmp/floating_base_model.urdf";
    doc->SaveFile(robot_urdf_file);

    load_robotmodel(robot_urdf_file, cfg.submechanism_file);
    jacobian.resize(6,noOfJoints());

    return true;
}

void RobotModelHyrodyn::update(const base::samples::Joints& joint_state,
                               const base::samples::RigidBodyStateSE3& _floating_base_state){

    RobotModel::update(joint_state, _floating_base_state);

    // Convert floating base state
    uint start_idx = 0;
    if(floating_base_robot){
        for(int i = 0; i < 6; i++){
            y(i)   = current_joint_state[floating_base_names[i]].position;
            yd(i)   = current_joint_state[floating_base_names[i]].speed;
            ydd(i)   = current_joint_state[floating_base_names[i]].acceleration;
        }
        start_idx = 6;
    }
    // Update KDL data types
    for( unsigned int i = start_idx; i < jointnames_independent.size(); ++i){
        const std::string& name =  jointnames_independent[i];
        y[i] = current_joint_state.getElementByName(name).position;
        yd[i] = current_joint_state.getElementByName(name).speed;
        ydd[i] = current_joint_state.getElementByName(name).acceleration;
        Tau_independentjointspace[i] = current_joint_state.getElementByName(name).effort;
    }

    calculate_com_properties();
    com_rbs.frame_id = base_frame;
    com_rbs.pose.position = com;
    com_rbs.pose.orientation.setIdentity();
    com_rbs.twist.linear = com_vel;
    com_rbs.twist.angular.setZero();
    com_rbs.acceleration.linear = com_acc;
    com_rbs.acceleration.angular.setZero();
    com_rbs.time = current_joint_state.time;
}

const base::samples::RigidBodyStateSE3 &RobotModelHyrodyn::rigidBodyState(const std::string &root_frame, const std::string &tip_frame){

    if(current_joint_state.time.isNull()){
        LOG_ERROR("RobotModelKDL: You have to call update() with appropriately timestamped joint data at least once before requesting kinematic information!");
        throw std::runtime_error(" Invalid call to rigidBodyState()");
    }

    if(root_frame != base_frame){
        LOG_ERROR_S<<"Requested Forward kinematics computation for kinematic chain "<<root_frame<<"->"<<tip_frame<<" but hyrodyn robot model always requires the root frame to be the root of the full model"<<std::endl;
        throw std::runtime_error("Invalid root frame");
    }

    calculate_forward_kinematics(tip_frame);
    rbs.pose.position        = pose.segment(0,3);
    rbs.pose.orientation     = base::Quaterniond(pose[6],pose[3],pose[4],pose[5]);
    rbs.twist.linear         = twist.segment(3,3);
    rbs.twist.angular        = twist.segment(0,3);
    rbs.acceleration.linear  = spatial_acceleration.segment(3,3);
    rbs.acceleration.angular = spatial_acceleration.segment(0,3);
    rbs.time                 = current_joint_state.time;
    rbs.frame_id             = tip_frame;
    return rbs;
}

const base::MatrixXd &RobotModelHyrodyn::spaceJacobian(const std::string &root_frame, const std::string &tip_frame){

    if(current_joint_state.time.isNull()){
        LOG_ERROR("RobotModelKDL: You have to call update() with appropriately timestamped joint data at least once before requesting kinematic information!");
        throw std::runtime_error(" Invalid call to rigidBodyState()");
    }

    if(root_frame != base_frame){
        LOG_ERROR_S<<"Requested Jacobian computation for kinematic chain "<<root_frame<<"->"<<tip_frame<<" but hyrodyn robot model always requires the root frame to be the root of the full model"<<std::endl;
        throw std::runtime_error("Invalid root frame");
    }

    calculate_space_jacobian(tip_frame);
    uint n_cols = Js.cols();
    jacobian.block(0,0,3,n_cols) = Js.block(3,0,3,n_cols);
    jacobian.block(3,0,3,n_cols) = Js.block(0,0,3,n_cols);
    return jacobian;
}

const base::MatrixXd &RobotModelHyrodyn::bodyJacobian(const std::string &root_frame, const std::string &tip_frame){

    if(current_joint_state.time.isNull()){
        LOG_ERROR("RobotModelKDL: You have to call update() with appropriately timestamped joint data at least once before requesting kinematic information!");
        throw std::runtime_error(" Invalid call to rigidBodyState()");
    }

    if(root_frame != base_frame){
        LOG_ERROR_S<<"Requested Jacobian computation for kinematic chain "<<root_frame<<"->"<<tip_frame<<" but hyrodyn robot model always requires the root frame to be the root of the full model"<<std::endl;
        throw std::runtime_error("Invalid root frame");
    }

    calculate_body_jacobian(tip_frame);
    uint n_cols = Js.cols();

    jacobian.block(0,0,3,n_cols) = Jb.block(3,0,3,n_cols);
    jacobian.block(3,0,3,n_cols) = Jb.block(0,0,3,n_cols);
    return jacobian;
}

const base::MatrixXd &RobotModelHyrodyn::jacobianDot(const std::string &root_frame, const std::string &tip_frame){

    throw std::runtime_error("Not implemented: jacobianDot has not been implemented for RobotModelHyrodyn");
}

const base::Acceleration &RobotModelHyrodyn::spatialAccelerationBias(const std::string &root_frame, const std::string &tip_frame){
    calculate_spatial_acceleration_bias(tip_frame);
    spatial_acc_bias = base::Acceleration(spatial_acceleration_bias.segment(3,3), spatial_acceleration_bias.segment(0,3));
    return spatial_acc_bias;
}

const base::MatrixXd &RobotModelHyrodyn::jointSpaceInertiaMatrix(){
    if(current_joint_state.time.isNull()){
        LOG_ERROR("RobotModelKDL: You have to call update() with appropriately timestamped joint data at least once before requesting kinematic information!");
        throw std::runtime_error(" Invalid call to rigidBodyState()");
    }

    calculate_mass_interia_matrix();
    joint_space_inertia_mat = H;
    return joint_space_inertia_mat;
}

const base::VectorXd &RobotModelHyrodyn::biasForces(){
    if(current_joint_state.time.isNull()){
        LOG_ERROR("RobotModelKDL: You have to call update() with appropriately timestamped joint data at least once before requesting kinematic information!");
        throw std::runtime_error(" Invalid call to rigidBodyState()");
    }

    ydd.setZero(); // TODO: Should be restored after the ID computation
    calculate_inverse_dynamics_independentjointspace();
    bias_forces = Tau_independentjointspace;
    return bias_forces;
}

}
