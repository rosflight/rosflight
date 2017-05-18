/**
 * \file rosflight_io.h
 * \author Daniel Koch <daniel.koch@byu.edu>
 */

#ifndef ROSFLIGHT_IO_MAVROSFLIGHT_ROS_H
#define ROSFLIGHT_IO_MAVROSFLIGHT_ROS_H

#include <map>
#include <string>

#include <ros/ros.h>

#include <std_msgs/Bool.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>

#include <sensor_msgs/Imu.h>
#include <sensor_msgs/FluidPressure.h>
#include <sensor_msgs/MagneticField.h>
#include <sensor_msgs/Temperature.h>
#include <sensor_msgs/Range.h>

#include <std_srvs/Trigger.h>

#include <rosflight_common/Attitude.h>
#include <rosflight_common/Barometer.h>
#include <rosflight_common/Airspeed.h>
#include <rosflight_common/Command.h>
#include <rosflight_common/OutputRaw.h>
#include <rosflight_common/RCRaw.h>
#include <rosflight_common/Status.h>

#include <rosflight_io/ParamFile.h>
#include <rosflight_io/ParamGet.h>
#include <rosflight_io/ParamSet.h>

#include <rosflight/mavrosflight/mavrosflight.h>
#include <rosflight/mavrosflight/mavlink_listener_interface.h>
#include <rosflight/mavrosflight/param_listener_interface.h>

#include <geometry_msgs/Quaternion.h>

namespace rosflight_io
{

class rosflightIO :
  public mavrosflight::MavlinkListenerInterface,
  public mavrosflight::ParamListenerInterface
{
public:
  rosflightIO();
  ~rosflightIO();

  virtual void handle_mavlink_message(const mavlink_message_t &msg);

  virtual void on_new_param_received(std::string name, double value);
  virtual void on_param_value_updated(std::string name, double value);
  virtual void on_params_saved_change(bool unsaved_changes);

private:

  // handle mavlink messages
  void handle_heartbeat_msg(const mavlink_message_t &msg);
  void handle_status_msg(const mavlink_message_t &msg);
  void handle_command_ack_msg(const mavlink_message_t &msg);
  void handle_statustext_msg(const mavlink_message_t &msg);
  void handle_attitude_quaternion_msg(const mavlink_message_t &msg);
  void handle_small_imu_msg(const mavlink_message_t &msg);
  void handle_rosflight_output_raw_msg(const mavlink_message_t &msg);
  void handle_rc_channels_raw_msg(const mavlink_message_t &msg);
  void handle_diff_pressure_msg(const mavlink_message_t &msg);
  void handle_small_baro_msg(const mavlink_message_t &msg);
  void handle_small_mag_msg(const mavlink_message_t &msg);
  void handle_named_value_int_msg(const mavlink_message_t &msg);
  void handle_named_value_float_msg(const mavlink_message_t &msg);
  void handle_named_command_struct_msg(const mavlink_message_t &msg);
  void handle_small_sonar(const mavlink_message_t &msg);
  void handle_version_msg(const mavlink_message_t &msg);

  // ROS message callbacks
  void commandCallback(rosflight_common::Command::ConstPtr msg);

  // ROS service callbacks
  bool paramGetSrvCallback(rosflight_io::ParamGet::Request &req, rosflight_io::ParamGet::Response &res);
  bool paramSetSrvCallback(rosflight_io::ParamSet::Request &req, rosflight_io::ParamSet::Response &res);
  bool paramWriteSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool paramSaveToFileCallback(rosflight_io::ParamFile::Request &req, rosflight_io::ParamFile::Response &res);
  bool paramLoadFromFileCallback(rosflight_io::ParamFile::Request &req, rosflight_io::ParamFile::Response &res);
  bool calibrateImuBiasSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool calibrateImuTempSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool calibrateRCTrimSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool calibrateMagSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool calibrateBaroSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool calibrateAirspeedSrvCallback(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
  bool rebootSrvCallback(std_srvs::Trigger::Request & req, std_srvs::Trigger::Response &res);

  // timer callbacks
  void paramTimerCallback(const ros::TimerEvent &e);
  void versionTimerCallback(const ros::TimerEvent &e);

  // helpers
  void request_version();

  template<class T> inline T saturate(T value, T min, T max)
  {
    return value < min ? min : (value > max ? max : value);
  }

  ros::NodeHandle nh_;

  ros::Subscriber command_sub_;

  ros::Publisher unsaved_params_pub_;
  ros::Publisher imu_pub_;
  ros::Publisher imu_temp_pub_;
  ros::Publisher output_raw_pub_;
  ros::Publisher rc_raw_pub_;
  ros::Publisher diff_pressure_pub_;
  ros::Publisher temperature_pub_;
  ros::Publisher baro_pub_;
  ros::Publisher sonar_pub_;
  ros::Publisher mag_pub_;
  ros::Publisher attitude_pub_;
  ros::Publisher euler_pub_;
  ros::Publisher status_pub_;
  ros::Publisher version_pub_;
  std::map<std::string, ros::Publisher> named_value_int_pubs_;
  std::map<std::string, ros::Publisher> named_value_float_pubs_;
  std::map<std::string, ros::Publisher> named_command_struct_pubs_;

  ros::ServiceServer param_get_srv_;
  ros::ServiceServer param_set_srv_;
  ros::ServiceServer param_write_srv_;
  ros::ServiceServer param_save_to_file_srv_;
  ros::ServiceServer param_load_from_file_srv_;
  ros::ServiceServer imu_calibrate_bias_srv_;
  ros::ServiceServer imu_calibrate_temp_srv_;
  ros::ServiceServer mag_calibrate_srv_;
  ros::ServiceServer calibrate_rc_srv_;
  ros::ServiceServer calibrate_baro_srv_;
  ros::ServiceServer calibrate_airspeed_srv_;
  ros::ServiceServer reboot_srv_;

  ros::Timer param_timer_;
  ros::Timer version_timer_;

  geometry_msgs::Quaternion attitude_quat_;
  uint8_t prev_status_;
  uint8_t prev_error_code_;
  uint8_t prev_control_mode_;

  std::string frame_id_;

  mavrosflight::MavROSflight *mavrosflight_;
  mavrosflight::sensors::Imu imu_;
  mavrosflight::sensors::Mag mag_;
};

} // namespace rosflight_io

#endif // ROSFLIGHT_IO_MAVROSFLIGHT_ROS_H
