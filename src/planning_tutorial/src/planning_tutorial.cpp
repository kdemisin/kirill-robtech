#include "ros/ros.h"
#include <cstdlib>
#include <moveit/move_group_interface/move_group.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/DisplayRobotState.h>
#include <moveit_msgs/DisplayTrajectory.h>


int main(int argc, char **argv)
{
  ros::init(argc, argv, "planning_tutorial");
  ros::NodeHandle node_handle;
  std::vector<geometry_msgs::Pose> waypoints;

moveit::planning_interface::MoveGroup group("sia5");
moveit::planning_interface::PlanningSceneInterface planning_scene_interface;

ros::Publisher display_publisher = node_handle.advertise<moveit_msgs::DisplayTrajectory>("/move_group/display_planned_path", 1, true);
moveit_msgs::DisplayTrajectory display_trajectory;

ROS_INFO("Reference frame: %s", group.getPlanningFrame().c_str());
ROS_INFO("Reference frame: %s", group.getEndEffectorLink().c_str());

robot_state::RobotState start_state(*group.getCurrentState());
geometry_msgs::Pose start_pose2;

geometry_msgs::Pose target_pose3 = start_pose2;

  geometry_msgs::Pose target_pose = start_pose2;
  target_pose3.position.x += 0.2;
  target_pose3.position.z += 0.2;
  waypoints.push_back(target_pose);  // up and out

  target_pose3.position.y -= 0.2;
  waypoints.push_back(target_pose);  // left

  target_pose3.position.z -= 0.2;
  target_pose3.position.y += 0.2;
  target_pose3.position.x -= 0.2;
  waypoints.push_back(target_pose);  // down and right (back to start)


//

moveit_msgs::RobotTrajectory trajectory;
double fraction = group.computeCartesianPath(waypoints,
                                             0.01,  // eef_step
                                             0.0,   // jump_threshold
                                             trajectory);

ROS_INFO("Visualizing plan 4 (cartesian path) (%.2f%% acheived)",
      fraction * 100.0);
/* Sleep to give Rviz time to visualize the plan. */
sleep(15.0);



/*
moveit::planning_interface::MoveGroup::Plan my_plan;
bool success = group.plan(my_plan);
group.execute(my_plan);

group.move(my_plan);


/*
group.setNamedTarget ("pose3");
group.move();
/*
sleep(15.0);
group.setNamedTarget ("home_pose");*/
//group.move();

/*
  pose =  moveit::planning_interface::MoveGroup::getCurrentPose(); 
  start_pose = poseA.pose; 
  


  geometry_msgs::Pose target_pose = start_pose;
  target_pose3.position.x += 0.2;
  target_pose3.position.z += 0.2;
  waypoints.push_back(target_pose);  // up and out

  target_pose3.position.y -= 0.2;
  waypoints.push_back(target_pose);  // left

  target_pose3.position.z -= 0.2;
  target_pose3.position.y += 0.2;
  target_pose3.position.x -= 0.2;
  waypoints.push_back(target_pose);  // down and right (back to start)



  moveit_msgs::RobotTrajectory trajectory;
  double fraction = group.computeCartesianPath(waypoints,
                                               0.01,  // eef_step
                                               0.0,   // jump_threshold
                                               trajectory);

  ROS_INFO("Visualizing plan 4 (cartesian path) (%.2f%% acheived)",
        fraction * 100.0);    

moveit::planning_interface::MoveGroup::execute(my_plan);
  
  
sleep(15.0);
*/

  return 0;
}
