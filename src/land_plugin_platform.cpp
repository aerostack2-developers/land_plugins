/*!*******************************************************************************************
 *  \file       land_plugin_platform.cpp
 *  \brief      land_plugin_platform implementation file
 *  \authors    Miguel Fernández Cortizas
 *              Pedro Arias Pérez
 *              David Pérez Saura
 *              Rafael Pérez Seguí
 *
 *  \copyright  Copyright (c) 2022 Universidad Politécnica de Madrid
 *              All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#include <std_srvs/srv/set_bool.hpp>
#include "land_base.hpp"
#include "motion_reference_handlers/hover_motion.hpp"

namespace land_plugin_platform
{
    class Plugin : public land_base::LandBase
    {
    public:
        rclcpp_action::GoalResponse onAccepted(const std::shared_ptr<const as2_msgs::action::Land::Goal> goal) override
        {
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }

        rclcpp_action::CancelResponse onCancel(const std::shared_ptr<GoalHandleLand> goal_handle) override
        {
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        bool onExecute(const std::shared_ptr<GoalHandleLand> goal_handle) override
        {
            rclcpp::Rate loop_rate(10);
            const auto goal = goal_handle->get_goal();
            auto feedback = std::make_shared<as2_msgs::action::Land::Feedback>();
            auto result = std::make_shared<as2_msgs::action::Land::Result>();

            static as2::motionReferenceHandlers::HoverMotion motion_handler_hover(node_ptr_);

            // Send platform takeoff service request
            auto request = std_srvs::srv::SetBool::Request();
            auto response = std_srvs::srv::SetBool::Response();
            request.data = true;

            auto takeoff_cli = as2::SynchronousServiceClient<std_srvs::srv::SetBool>(
                as2_names::services::platform::land);

            bool out = takeoff_cli.sendRequest(request, response);

            if (!(out && response.success))
            {
                result->land_success = false;
                goal_handle->canceled(result);
                RCLCPP_ERROR(node_ptr_->get_logger(),
                             "Platform land service request failed");
                motion_handler_hover.sendHover();
                return false;
            }


            result->land_success = true;
            goal_handle->succeed(result);
            RCLCPP_INFO(node_ptr_->get_logger(), "Goal succeeded");
            return true;
        }

    }; // Plugin class
} // namespace land_plugin_platform

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(land_plugin_platform::Plugin, land_base::LandBase)
