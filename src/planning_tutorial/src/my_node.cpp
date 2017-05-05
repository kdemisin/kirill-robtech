#include "ros/ros.h"
#include <cstdlib>
#include <moveit/move_group_interface/move_group.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>



int main(int argc, char **argv)
{
  ros::init(argc, argv, "my_node");
  ros::NodeHandle n;
int count = 0;
  moveit::planning_interface::MoveGroup group("right_arm");
  moveit::planning_interface::PlanningSceneInterface planning_scene_interface;
  


  return 0;
}
