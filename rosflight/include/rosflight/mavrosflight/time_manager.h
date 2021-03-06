/*
 * Copyright (c) 2017 Daniel Koch and James Jackson, BYU MAGICC Lab.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file time_manager.h
 * \author Daniel Koch <daniel.koch@byu.edu>
 */

#ifndef MAVROSFLIGHT_TIME_MANAGER_H
#define MAVROSFLIGHT_TIME_MANAGER_H

#include <rosflight/mavrosflight/logger_interface.h>
#include <rosflight/mavrosflight/mavlink_bridge.h>
#include <rosflight/mavrosflight/mavlink_comm.h>
#include <rosflight/mavrosflight/mavlink_listener_interface.h>
#include <rosflight/mavrosflight/time_interface.h>
#include <rosflight/mavrosflight/timer_interface.h>

#include <chrono>
#include <memory>

namespace mavrosflight
{
template <typename DerivedLogger>
class TimeManager : MavlinkListenerInterface
{
public:
  TimeManager(MavlinkComm *comm,
              LoggerInterface<DerivedLogger> &logger,
              const TimeInterface &time_interface,
              TimerProviderInterface &timer_provider);

  virtual void handle_mavlink_message(const mavlink_message_t &msg);

  std::chrono::nanoseconds fcu_time_to_system_time(std::chrono::nanoseconds fcu_time);

private:
  MavlinkComm *comm_;

  std::shared_ptr<TimerInterface> time_sync_timer_;
  void timer_callback();

  double offset_alpha_;
  std::chrono::nanoseconds offset_ns_;

  bool initialized_;

  LoggerInterface<DerivedLogger> &logger_;
  const TimeInterface &time_interface_;
  TimerProviderInterface &timer_provider_;
};

} // namespace mavrosflight

#endif // MAVROSFLIGHT_TIME_MANAGER_H
