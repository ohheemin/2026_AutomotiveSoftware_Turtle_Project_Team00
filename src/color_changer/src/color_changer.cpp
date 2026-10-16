#include "color_changer.hpp"


ColorChanger::ColorChanger(const std::string& node_name, const double& loop_rate)
    : Node(node_name)  {
    
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    
    // -------------------TODO - define publisher and subscriber--------------------
    s_turtle_pose_ = this->create_subscription<turtlesim::msg::Pose>(
        "turtle1/pose", qos_profile,
        std::bind(&ColorChanger::CallbackTurtlePose, this, std::placeholders::_1));
    
    p_turtle_color_ = this->create_publisher<my_msgs::msg::TurtleColor>(
        "turtle_color", qos_profile
    );
    // -------------------------------------------------------------------------------

    // parameter client to change the background color parameters of the turtlesim node
    param_client_ = std::make_shared<rclcpp::AsyncParametersClient>(this, "/turtlesim");

    t_run_node_ = this->create_wall_timer(
            std::chrono::microseconds((int64_t)(1e6 / loop_rate)),
            [this]()
            { this->Run(this->now()); });
}


void ColorChanger::Run(const rclcpp::Time &current_time) {
    int background_r = 0;   // 0 ~ 255
    int background_g = 0;
    int background_b = 0;

    // -------------------TODO - set background_r/g/b and o_turtle_color_ according to the turtle's yaw angle--------------------
    o_turtle_color_.red   = 0.0;   // HMI로 보낼 색을 먼저 0으로 초기화 (멤버 변수라 이전 값이 남기 때문)
    o_turtle_color_.green = 0.0;
    o_turtle_color_.blue  = 0.0;
    
    double turtle_degree = i_turtle_pose_.theta * 180.0 / M_PI;
    if (turtle_degree >= 180) {
        turtle_degree -= 360;
    } else if(turtle_degree < -180) {
        turtle_degree += 360;
    }

    if (turtle_degree < -90) {
        background_g = 255;
        o_turtle_color_.green = 1.0;
    } else if (turtle_degree < 0) {
        background_b = 255;
        o_turtle_color_.blue  = 1.0;
    } else if (turtle_degree < 90) {
        background_r = 255; 
        background_g = 255;
        background_b = 255;

        o_turtle_color_.red   = 1.0;  
        o_turtle_color_.green = 1.0;
        o_turtle_color_.blue  = 1.0;
    } else {
        background_r = 255;
        o_turtle_color_.red   = 1.0;
    }
    // -------------------------------------------------------------------------------

    // set the background color parameters of the turtlesim node
    params_.clear();
    params_.push_back(rclcpp::Parameter("background_r", background_r));
    params_.push_back(rclcpp::Parameter("background_g", background_g));
    params_.push_back(rclcpp::Parameter("background_b", background_b));
    param_client_->set_parameters(params_);

    Publish(current_time);
}

void ColorChanger::Publish(const rclcpp::Time &current_time) {
    p_turtle_color_->publish(o_turtle_color_);
}


int main(int argc, char *argv[]) {
    std::string node_name = "color_changer";
    double loop_rate = 10.0;

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ColorChanger>(node_name, loop_rate));
    rclcpp::shutdown();
    return 0;
}
