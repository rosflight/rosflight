/*
 * Software License Agreement (BSD-3 License)
 *
 * Copyright (c) 2020 Jacob Willis.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file ros_timer.h
 * @author Jacob Willis <jbwillis272@gmail.com>
 */

#ifndef ROSFLIGHT_ROS_TIMER_H
#define ROSFLIGHT_ROS_TIMER_H

#include <rosflight/mavrosflight/timer_interface.h>

#include <ros/ros.h>

#include <vector>

namespace rosflight
{
class ROSTimer : public mavrosflight::AbstractTimer
{
public:
  inline ROSTimer(ros::Timer timer) : timer_(timer) {}

  inline void start() { timer_.start(); }

  inline void stop() { timer_.stop(); }

private:
  ros::Timer timer_;
};

class ROSTimerInterface : public mavrosflight::TimerInterface<ROSTimerInterface>
{
public:
  template <class T>
  inline mavrosflight::AbstractTimer* createTimer(uint32_t rate_hz,
                                                  void (T::*callback)(),
                                                  T* obj,
                                                  bool oneshot = false,
                                                  bool autostart = true)
  {
    ros::NodeHandle nh;
    ros::Timer new_timer;
    auto callback_ros = [&](ros::TimerEvent& event) { (obj->*callback)(); };

    new_timer = nh.createTimer(ros::Duration(ros::Rate(rate_hz)), callback_ros, oneshot, autostart);
    ROSTimer* new_ros_timer = new ROSTimer(new_timer);
    timer_vec_.push_back(new_ros_timer);
    return new_ros_timer;
  }

  inline ~ROSTimerInterface()
  {
    for (ROSTimer* rt : timer_vec_)
    {
      delete rt;
    }
  }

private:
  std::vector<ROSTimer*> timer_vec_;
};

} // namespace rosflight
#endif /* ROSFLIGHT_ROS_TIMER_H */