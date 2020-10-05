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
  time_interface_(time_interface),
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
 * @file timer_interface.h
 * @author Jacob Willis <jbwillis272@gmail.com>
 */

#ifndef MAVROSFLIGHT_TIMER_INTERFACE_H
#define MAVROSFLIGHT_TIMER_INTERFACE_H

#include <chrono>
#include <functional>
#include <memory>

namespace mavrosflight
{
/**
 * \class AbstractTimer
 * \brief Abstracts basic timer functionality
 *
 */
class AbstractTimer
{
public:
  virtual void start() = 0;
  virtual void stop() = 0;
};

/**
 * \class TimerInterface
 * \brief Provide an interface for creating timers
 */
class TimerInterface
{
public:
  virtual std::shared_ptr<AbstractTimer> createTimer(std::chrono::nanoseconds period,
                                                     std::function<void()> callback,
                                                     const bool oneshot = false,
                                                     const bool autostart = true) = 0;
};

} // namespace mavrosflight
#endif /* MAVROSFLIGHT_TIMER_INTERFACE_H */
