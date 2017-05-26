// Copyright (c) 2015-2016, The University of Texas at Austin
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
// 
//     * Neither the name of the copyright holder nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/** @file griffin_powermate.h
 *  On /griffin_powermate/events topic it publishes griffin_powermate::PowermateEvent
 *  messages that contain direction and integral of the turn wheel as well as the 
 *  information about push button being pressed or depressed.
 * 
 *  NOTE
 *  If you get permission denied when running this ROS node, use
 * 	ls -l /dev/input/event*
 *  to learn which group can access linux input events. Then add your username to
 *  that group by issuing
 *  	sudo usermod -a -G [group_name] [user_name]
 *  You need to log out and back in for these changes to take effect.
 * 
 *  @author karl.kruusamae(at)utexas.edu
 */

#include "mouse_reader/mouse_reader.h"

#include <visualization_msgs/Marker.h>

/** Opens the input device and checks whether its meaningful name (ie, EVIOCGNAME in ioctl()) is listed in valid_substrings_.
 *  @param device_path file name for linux event.
 *  @return file descriptor to PowerMate event if all checks out, otherwise -1.
 */

int main(int argc, char *argv[])
{

  // ROS init
  ros::init(argc, argv, "mouse_reader");
  ros::NodeHandle n;
  ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 1);
uint32_t shape = visualization_msgs::Marker::SPHERE;// Private nodehandle for ROS
  ros::NodeHandle pnh("~");
  visualization_msgs::Marker marker;
    // Set the frame ID and timestamp.  See the TF tutorials for information on these.
    marker.header.frame_id = "/my_frame";
    marker.header.stamp = ros::Time::now();

    // Set the namespace and id for this marker.  This serves to create a unique ID
    // Any marker sent with the same namespace and id will overwrite the old one
    marker.ns = "basic_shapes";
    marker.id = 0;

    // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
    marker.type = shape;

    // Set the marker action.  Options are ADD, DELETE, and new in ROS Indigo: 3 (DELETEALL)
    marker.action = visualization_msgs::Marker::ADD;

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
    marker.pose.position.x = 0;
    marker.pose.position.y = 0;
    marker.pose.position.z = 0;
    marker.pose.orientation.x = 0.0;
    marker.pose.orientation.y = 0.0;
    marker.pose.orientation.z = 0.0;
    marker.pose.orientation.w = 1.0;

    // Set the scale of the marker -- 1x1x1 here means 1m on a side
    marker.scale.x = 1.0;
    marker.scale.y = 1.0;
    marker.scale.z = 1.0;

    // Set the color -- be sure to set alpha to something non-zero!
    marker.color.r = 0.0f;
    marker.color.g = 1.0f;
    marker.color.b = 0.0f;
    marker.color.a = 1.0;

    marker.lifetime = ros::Duration();

switch (shape)
    {
    case visualization_msgs::Marker::CUBE:
      shape = visualization_msgs::Marker::SPHERE;
      break;
    case visualization_msgs::Marker::SPHERE:
      shape = visualization_msgs::Marker::ARROW;
      break;
    case visualization_msgs::Marker::ARROW:
      shape = visualization_msgs::Marker::CYLINDER;
      break;
    case visualization_msgs::Marker::CYLINDER:
      shape = visualization_msgs::Marker::CUBE;
      break;
    }
  // Getting user-specified path from ROS parameter server
  std::string powermate_path;
  pnh.param<std::string>("path", powermate_path, "");
  
  // Let's construct PowerMate object 
  PowerMate powermate(powermate_path);
  
  // If failed to open any PowerMate USB device, print info and exit
  if( !powermate.isReadable() )
  {
    ROS_ERROR("Unable to locate any PowerMate device.");
    ROS_INFO("You may try specifying path as ROS parameter, e.g., rosrun mouse_reader mouse_reader _path:=<device_event_path>");
    return -1;
  }

  // Creates publisher that advertises griffin_powermate::PowermateEvent messages on topic /griffin_powermate/events
  ros::Publisher pub_My_events = pnh.advertise<mouse_reader::MyEvent>("events", 100);
  
  // After PowerMate is succesfully opened, read its input, publish ROS messages, and spin.
  powermate.spinPowerMate(pub_My_events);

  // Close PowerMate
  powermate.closePowerMate();

  return 0;
} //end main

int PowerMate::openPowerMate(const char *device_path)
{
  printf("Opening device: %s \n", device_path);
  
  // Open device at device_path for READONLY and get file descriptor 
  int fd = open(device_path, O_RDONLY);
  
  // If failed to open device at device_path
  if(fd < 0)
  {
    ROS_ERROR("Failed to open \"%s\"\n", device_path);
    return -1;
  }

 // Meaningful, i.e., EVIOCGNAME name
  char name[255];
  // Fetch the meaningful (i.e., EVIOCGNAME) name
  if(ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
  {
    ROS_ERROR("\"%s\": EVIOCGNAME failed.", device_path);
    close(fd);
    // Returns -1 if failed to fetch the meaningful name
    return -1;
  }

  // Let's check if the meaningful name matches one listed in valid_substrings_
  std::ostringstream sstream;
  // Convert name given as char* to stringstream
  sstream << name;
  // stringstream to string
  std::string name_as_string = sstream.str();
  int i;
  for (i=0; i < valid_substrings_.size(); i++)
  {
    // Does the meaningful name contain a predefined valid substring?
    std::size_t found = name_as_string.find( valid_substrings_[i] );
    if (found!=std::string::npos)
    {
      // if everything checks out, print on screen and return the file descriptor
      ROS_INFO("Found \x1b[1;34m'%s'\x1b[0m device. Starting to read ...\n", name);
      return fd;
    } // end if
  } // end for
  
  close(fd);
  return -1;
} // end openPowerMate



/** Goes through all the event files in /dev/input/ to locate Griffin PowerMate USB.
 *  @return file descriptor if all checks out, otherwise -1.
 */
int PowerMate::findPowerMate()
{
  // Using glob() [see: http://linux.die.net/man/3/glob ] for getting event files in /dev/input/
  glob_t gl;
  // Number of event files found in /dev/input/
  int num_event_dev = 0;
  // Counts for filenames that match the given pattern
  if(glob("/dev/input/event*", GLOB_NOSORT, NULL, &gl) == 0)
  {
    // Get number of event files
    num_event_dev = gl.gl_pathc;
  }

  int i, r;
  // Goes through all the found event files
  for(i = 0; i < num_event_dev; i++)
  {
    // Tries to open an event file as a PowerMate device
    r = openPowerMate(gl.gl_pathv[i]);
    // If opened file is PowerMate event, return file descriptor
    if(r >= 0) return r;
  } // for
  
  // free memory allocated for globe struct
  globfree(&gl);

  // return error -1 because no PowerMate device was found
  return -1;
} // end findPowerMate

/** Closes the device specificed by descriptor_. */
void PowerMate::closePowerMate()
{
  printf("Closing PowerMate device.\n");
  close(descriptor_);
  return;
}

/** Checks if the PowerMate event file has been succesfully opened.
 *  @return TRUE when descriptor_ is not negative, FALSE otherwise.
 */
bool PowerMate::isReadable ()
{
  if (descriptor_ < 0) return false;
  return true;
}

/** Processes the event data and publishes it as PowermateEvent message.
 *  @param ev input event.
 *  @param ros_publisher ROS publisher.
 */
void PowerMate::processEvent(struct input_event *ev, ros::Publisher& ros_publisher)
{
  // PowermateEvent ROS message
ros::NodeHandle n;
 ros::Rate r(1);
  mouse_reader::MyEvent ros_message;
visualization_msgs::Marker marker;
  ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 1);
uint32_t shape = visualization_msgs::Marker::ARROW;

    // Set the frame ID and timestamp.  See the TF tutorials for information on these.
    marker.header.frame_id = "/my_frame";
    marker.header.stamp = ros::Time::now();

    // Set the namespace and id for this marker.  This serves to create a unique ID
    // Any marker sent with the same namespace and id will overwrite the old one
    marker.ns = "basic_shapes";
    marker.id = 0;

    // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
    marker.type = shape;

    // Set the marker action.  Options are ADD, DELETE, and new in ROS Indigo: 3 (DELETEALL)
    

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
    marker.pose.position.x = 0;
    marker.pose.position.y = 0;
    marker.pose.position.z = 0;
    marker.pose.orientation.x = 0.0;
    marker.pose.orientation.y = 0.0;
    marker.pose.orientation.z = 0.0;
    marker.pose.orientation.w = 1.0;

    // Set the scale of the marker -- 1x1x1 here means 1m on a side
    marker.scale.x = 1.0;
    marker.scale.y = 1.0;
    marker.scale.z = 1.0;

    // Set the color -- be sure to set alpha to something non-zero!
    marker.color.r = 0.0f;
    marker.color.g = 1.0f;
    marker.color.b = 0.0f;
    marker.color.a = 1.0;

    marker.lifetime = ros::Duration();

marker_pub.publish(marker);
switch (shape)
    {
    case visualization_msgs::Marker::CUBE:
      shape = visualization_msgs::Marker::SPHERE;
      break;
    case visualization_msgs::Marker::SPHERE:
      shape = visualization_msgs::Marker::ARROW;
      break;
    case visualization_msgs::Marker::ARROW:
      shape = visualization_msgs::Marker::CYLINDER;
      break;
    case visualization_msgs::Marker::CYLINDER:
      shape = visualization_msgs::Marker::CUBE;
      break;
    

    default:					// default case
      ROS_WARN("Unexpected event type; ev->type = 0x%04x\n", ev->type);
  } // end switch
marker.action = visualization_msgs::Marker::ADD;

r.sleep();
  fflush(stdout);
} // end processEvent

/** Method for reading the event data and ROS spinning.
 *  @param ros_publisher ROS publisher used to publish PowermateEvent message.
 */
void PowerMate::spinPowerMate(ros::Publisher& ros_publisher)
{
  int const BUFFER_SIZE = 32;
  uint32_t shape = visualization_msgs::Marker::ARROW;
  // see: https://www.kernel.org/doc/Documentation/input/input.txt
  struct input_event ibuffer[BUFFER_SIZE];
  int r, events, i;

  while( ros::ok() )
  {  
   visualization_msgs::Marker marker;
    // Set the frame ID and timestamp.  See the TF tutorials for information on these.
    marker.header.frame_id = "/my_frame";
    marker.header.stamp = ros::Time::now();

    // Set the namespace and id for this marker.  This serves to create a unique ID
    // Any marker sent with the same namespace and id will overwrite the old one
    marker.ns = "basic_shapes";
    marker.id = 0;

    // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
    marker.type = shape;

    // Set the marker action.  Options are ADD, DELETE, and new in ROS Indigo: 3 (DELETEALL)
    marker.action = visualization_msgs::Marker::ADD;

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
    marker.pose.position.x = 0;
    marker.pose.position.y = 0;
    marker.pose.position.z = 0;
    marker.pose.orientation.x = 0.0;
    marker.pose.orientation.y = 0.0;
    marker.pose.orientation.z = 0.0;
    marker.pose.orientation.w = 1.0;

    // Set the scale of the marker -- 1x1x1 here means 1m on a side
    marker.scale.x = 1.0;
    marker.scale.y = 1.0;
    marker.scale.z = 1.0;

    // Set the color -- be sure to set alpha to something non-zero!
    marker.color.r = 0.0f;
    marker.color.g = 1.0f;
    marker.color.b = 0.0f;
    marker.color.a = 1.0;

    marker.lifetime = ros::Duration();

switch (shape)
    {
    case visualization_msgs::Marker::CUBE:
      shape = visualization_msgs::Marker::SPHERE;
      break;
    case visualization_msgs::Marker::SPHERE:
      shape = visualization_msgs::Marker::ARROW;
      break;
    case visualization_msgs::Marker::ARROW:
      shape = visualization_msgs::Marker::CYLINDER;
      break;
    case visualization_msgs::Marker::CYLINDER:
      shape = visualization_msgs::Marker::CUBE;
      break;
    }
    r = read(descriptor_, ibuffer, sizeof(struct input_event) * BUFFER_SIZE);
    if( r > 0 )
    {
      // Calculate the number of events
      events = r / sizeof(struct input_event);
      // Go through each read events
      for(i = 0; i < events; i++)
      {
	// Process event and publish data
uint32_t shape = visualization_msgs::Marker::CYLINDER;
	processEvent(&ibuffer[i], ros_publisher);
	// spin
	ros::spinOnce();
      } // end for
    } // end if
    else
    {
      // Let user know if read() has failed
      ROS_WARN("read() failed.\n");
      return;
    } // end else

  } // end while
  
  return;
} // end spinPowerMate

/** Main method. */

