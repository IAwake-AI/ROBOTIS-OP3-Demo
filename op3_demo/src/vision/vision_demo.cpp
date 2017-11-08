/*******************************************************************************
* Copyright 2017 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Author: Kayman Jung */

#include "op3_demo/vision_demo.h"

namespace robotis_op
{

VisionDemo::VisionDemo()
    : SPIN_RATE(30),
      is_tracking_(false),
      tracking_status_(FaceTracker::Waiting)
{
  enable_ = false;

  ros::NodeHandle nh(ros::this_node::getName());

  boost::thread queue_thread = boost::thread(boost::bind(&VisionDemo::callbackThread, this));
  boost::thread process_thread = boost::thread(boost::bind(&VisionDemo::processThread, this));
}

VisionDemo::~VisionDemo()
{
  // TODO Auto-generated destructor stub
}

void VisionDemo::setDemoEnable()
{
  // change to motion module
  setModuleToDemo("action_module");

  usleep(100 * 1000);

  playMotion(InitPose);

  usleep(1500 * 1000);

  setModuleToDemo("head_control_module");

  usleep(10 * 1000);

  enable_ = true;
  face_tracker_.startTracking();

  ROS_INFO("Start Vision Demo");

}

void VisionDemo::setDemoDisable()
{

  face_tracker_.stopTracking();
  is_tracking_ = false;
  tracking_status_ = FaceTracker::Waiting;
  enable_ = false;
}

void VisionDemo::process()
{
  int tracking_status = face_tracker_.processTracking();

  if(tracking_status_ != tracking_status)
  {
    switch(tracking_status)
    {
      case FaceTracker::Found:
        setRGBLED(0x1F, 0x1F, 0x1F);
        break;

      case FaceTracker::NotFound:
        setRGBLED(0, 0, 0);
        break;

      default:
        break;
    }
  }

  if(tracking_status != FaceTracker::Waiting)
    tracking_status_ = tracking_status;

  //is_tracking_ = is_tracked;
  std::cout << "Tracking : " << tracking_status << std::endl;
}

void VisionDemo::processThread()
{
  //set node loop rate
  ros::Rate loop_rate(SPIN_RATE);

  //node loop
  while (ros::ok())
  {
    if (enable_ == true)
      process();

    //relax to fit output rate
    loop_rate.sleep();
  }
}

void VisionDemo::callbackThread()
{
  ros::NodeHandle nh(ros::this_node::getName());

  // subscriber & publisher
  module_control_pub_ = nh.advertise<std_msgs::String>("/robotis/enable_ctrl_module", 0);
  motion_index_pub_ = nh.advertise<std_msgs::Int32>("/robotis/action/page_num", 0);
  rgb_led_pub_ = nh.advertise<robotis_controller_msgs::SyncWriteItem>("/robotis/sync_write_item", 0);

  buttuon_sub_ = nh.subscribe("/robotis/open_cr/button", 1, &VisionDemo::buttonHandlerCallback, this);
  faceCoord_sub_ = nh.subscribe("/faceCoord", 1, &VisionDemo::facePositionCallback, this);

  while (nh.ok())
  {
    ros::spinOnce();

    usleep(1 * 1000);
  }
}

void VisionDemo::buttonHandlerCallback(const std_msgs::String::ConstPtr& msg)
{
  if (enable_ == false)
    return;

  if (msg->data == "start")
  {
//    switch (play_status_)
//      {
//        case PlayAction:
//        {
//          pauseProcess();
//          break;
//        }
//
//        case PauseAction:
//        {
//          resumeProcess();
//          break;
//        }
//
//        case StopAction:
//        {
//          resumeProcess();
//          break;
//        }
//
//        default:
//          break;
//      }
  }
  else if (msg->data == "mode")
  {

  }
}

void VisionDemo::setModuleToDemo(const std::string &module_name)
{
  std_msgs::String control_msg;
  control_msg.data = module_name;

  module_control_pub_.publish(control_msg);
  std::cout << "enable module : " << module_name << std::endl;
}

void VisionDemo::facePositionCallback(const std_msgs::Int32MultiArray::ConstPtr &msg)
{
  if (enable_ == false)
    return;

  // face is detected
  if (msg->data.size() >= 10)
  {
    // center of face
    face_position_.x = (msg->data[6] + msg->data[8] * 0.5) / msg->data[2] * 2 - 1;
    face_position_.y = (msg->data[7] + msg->data[9] * 0.5) / msg->data[3] * 2 - 1;
    face_position_.z = msg->data[8] * 0.5 + msg->data[9] * 0.5;

    face_tracker_.setFacePosition(face_position_);
  }
  else
  {
    face_position_.x = 0;
    face_position_.y = 0;
    face_position_.z = 0;
    return;
  }
}

void VisionDemo::playMotion(int motion_index)
{
  std_msgs::Int32 motion_msg;
  motion_msg.data = motion_index;

  motion_index_pub_.publish(motion_msg);
}

void VisionDemo::setRGBLED(int blue, int green, int red)
{
  int led_full_unit = 0x1F;
  int led_value = (blue & led_full_unit) << 10 | (green & led_full_unit) << 5 | (red & led_full_unit);
  robotis_controller_msgs::SyncWriteItem syncwrite_msg;
  syncwrite_msg.item_name = "LED_RGB";
  syncwrite_msg.joint_name.push_back("open-cr");
  syncwrite_msg.value.push_back(led_value);

  rgb_led_pub_.publish(syncwrite_msg);
}

} /* namespace robotis_op */
