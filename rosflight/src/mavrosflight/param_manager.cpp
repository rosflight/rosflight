/**
 * \file param_manager.h
 * \author Daniel Koch <daniel.koch@byu.edu>
 */

#include <rosflight/mavrosflight/param_manager.h>
#include <ros/ros.h>
#include <yaml-cpp/yaml.h>

#include <fstream>

namespace mavrosflight
{

ParamManager::ParamManager(MavlinkSerial * const serial) :
  serial_(serial),
  unsaved_changes_(false),
  write_request_in_progress_(false),
  first_param_received_(false),
  received_count_(0),
  got_all_params_(false)
{
  serial_->register_mavlink_listener(this);
}

ParamManager::~ParamManager()
{
  if (first_param_received_)
  {
    delete[] received_;
  }
}

void ParamManager::handle_mavlink_message(const mavlink_message_t &msg)
{
  switch (msg.msgid)
  {
  case MAVLINK_MSG_ID_PARAM_VALUE:
    handle_param_value_msg(msg);
    break;
  case MAVLINK_MSG_ID_ROSFLIGHT_CMD_ACK:
    handle_command_ack_msg(msg);
    break;
  }
}

bool ParamManager::unsaved_changes()
{
  return unsaved_changes_;
}

bool ParamManager::get_param_value(std::string name, double *value)
{
  if (is_param_id(name))
  {
    *value = params_[name].getValue();
    return true;
  }
  else
  {
    *value = 0.0;
    return false;
  }
}

bool ParamManager::set_param_value(std::string name, double value)
{
  if (is_param_id(name))
  {
    mavlink_message_t msg;
    params_[name].requestSet(value, &msg);
    serial_->send_message(msg);

    return true;
  }
  else
  {
    return false;
  }
}

bool ParamManager::write_params()
{
  if (!write_request_in_progress_)
  {
    mavlink_message_t msg;
    uint8_t sysid = 1;
    uint8_t compid = 1;
    mavlink_msg_rosflight_cmd_pack(sysid, compid, &msg, ROSFLIGHT_CMD_WRITE_PARAMS);
    serial_->send_message(msg);

    write_request_in_progress_ = true;
    return true;
  }
  else
  {
    return false;
  }
}

void ParamManager::register_param_listener(ParamListenerInterface *listener)
{
  if (listener == NULL)
    return;

  bool already_registered = false;
  for (int i = 0; i < listeners_.size(); i++)
  {
    if (listener == listeners_[i])
    {
      already_registered = true;
      break;
    }
  }

  if (!already_registered)
    listeners_.push_back(listener);
}

void ParamManager::unregister_param_listener(ParamListenerInterface *listener)
{
  if (listener == NULL)
    return;

  for (int i = 0; i < listeners_.size(); i++)
  {
    if (listener == listeners_[i])
    {
      listeners_.erase(listeners_.begin() + i);
      i--;
    }
  }
}

bool ParamManager::save_to_file(std::string filename)
{
  // build YAML document
  YAML::Emitter yaml;
  yaml << YAML::BeginSeq;
  std::map<std::string, Param>::iterator it;
  for (it = params_.begin(); it != params_.end(); it++)
  {
    yaml << YAML::Flow;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "name" << YAML::Value << it->second.getName();
    yaml << YAML::Key << "type" << YAML::Value << (int) it->second.getType();
    yaml << YAML::Key << "value" << YAML::Value << it->second.getValue();
    yaml << YAML::EndMap;
  }
  yaml << YAML::EndSeq;

  // write to file
  try
  {
    std::ofstream fout;
    fout.open(filename.c_str());
    fout << yaml.c_str();
    fout.close();
  }
  catch (...)
  {
    return false;
  }

  return true;
}

bool ParamManager::load_from_file(std::string filename)
{
  try
  {
    YAML::Node root = YAML::LoadFile(filename);
    assert(root.IsSequence());

    for (int i = 0; i < root.size(); i++)
    {
      if (root[i].IsMap() && root[i]["name"] && root[i]["type"] && root[i]["value"])
      {
        if (is_param_id(root[i]["name"].as<std::string>()))
        {
          Param param = params_.find(root[i]["name"].as<std::string>())->second;
          if ((MAV_PARAM_TYPE) root[i]["type"].as<int>() == param.getType())
          {
            set_param_value(root[i]["name"].as<std::string>(), root[i]["value"].as<double>());
          }
        }
      }
    }

    return true;
  }
  catch (...)
  {
    return false;
  }
}

void ParamManager::request_params()
{
  if (!first_param_received_)
  {
    request_param_list();
  }
  else
  {
    for (size_t i = 0; i < num_params_; i++)
    {
      if (!received_[i])
      {
        request_param(i);
      }
    }
  }
}

void ParamManager::request_param_list()
{
  mavlink_message_t param_list_msg;
  mavlink_msg_param_request_list_pack(1, 50, &param_list_msg, 1, MAV_COMP_ID_ALL);
  serial_->send_message(param_list_msg);
}

void ParamManager::request_param(int index)
{
  mavlink_message_t param_request_msg;
  char empty[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN];
  mavlink_msg_param_request_read_pack(1, 50, &param_request_msg, 1, MAV_COMP_ID_ALL, empty, (int16_t) index);
  serial_->send_message(param_request_msg);
}

void ParamManager::handle_param_value_msg(const mavlink_message_t &msg)
{
  mavlink_param_value_t param;
  mavlink_msg_param_value_decode(&msg, &param);

  if (!first_param_received_)
  {
    first_param_received_ = true;
    num_params_ = param.param_count;
    received_ = new bool[num_params_];
    for (int i = 0; i < num_params_; i++)
    {
      received_[i] = false;
    }
  }

  // ensure null termination of name
  char c_name[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN + 1];
  memcpy(c_name, param.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN);
  c_name[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN] = '\0';

  std::string name(c_name);

  if (!is_param_id(name)) // if we haven't received this param before, add it
  {
    params_[name] = Param(param);
    received_[param.param_index] = true;

    // increase the param count
    received_count_++;
    if(received_count_ == num_params_)
    {
      got_all_params_ = true;
    }

    for (int i = 0; i < listeners_.size(); i++)
      listeners_[i]->on_new_param_received(name, params_[name].getValue());
  }
  else // otherwise check if we have new unsaved changes as a result of a param set request
  {
    if (params_[name].handleUpdate(param))
    {
      unsaved_changes_ = true;
      for (int i = 0; i < listeners_.size(); i++)
      {
        listeners_[i]->on_param_value_updated(name, params_[name].getValue());
        listeners_[i]->on_params_saved_change(unsaved_changes_);
      }
    }
  }
}

void ParamManager::handle_command_ack_msg(const mavlink_message_t &msg)
{
  if (write_request_in_progress_)
  {
    mavlink_rosflight_cmd_ack_t ack;
    mavlink_msg_rosflight_cmd_ack_decode(&msg, &ack);

    if (ack.command == ROSFLIGHT_CMD_WRITE_PARAMS)
    {
      write_request_in_progress_ = false;
      if(ack.success == ROSFLIGHT_CMD_SUCCESS)
      {
        ROS_INFO("Param write succeeded");
        unsaved_changes_ = false;

        for (int i = 0; i < listeners_.size(); i++)
          listeners_[i]->on_params_saved_change(unsaved_changes_);
      }
      else
      {
        ROS_INFO("Param write failed - maybe disarm the aricraft and try again?");
        write_request_in_progress_ = false;
        unsaved_changes_ = true;
      }
    }
  }
}

bool ParamManager::is_param_id(std::string name)
{
  return (params_.find(name) != params_.end());
}

int ParamManager::get_num_params()
{
  if (first_param_received_)
  {
    return num_params_;
  }
  else
  {
    return 0;
  }
}

int ParamManager::get_params_received()
{
  return received_count_;
}

bool ParamManager::got_all_params()
{
  return got_all_params_;
}

} // namespace mavrosflight
