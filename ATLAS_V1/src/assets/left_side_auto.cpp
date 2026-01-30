#include "lemlib/api.hpp" // IWYU pragma: keep
#include "main.h"
#include "liblvgl/display/lv_display.h"
#include "liblvgl/misc/lv_area.h"
#include "liblvgl/widgets/image/lv_image.h"



//Controller
pros::Controller master(pros::E_CONTROLLER_MASTER);

//Drivetrain motorgroups
pros::MotorGroup left_mg({-13, -6, 5}, pros::MotorGearset::blue);
pros::MotorGroup right_mg({14, 1, -12}, pros::MotorGearset::blue);
//right_mg.set_gearing(pros::E_MOTOR_GEARSET_06);
//left_mg.set_gearing(pros::E_MOTOR_GEARSET_06);

// drivetrain settings
lemlib::Drivetrain Drivetrain(&left_mg, // left motor group
                              &right_mg, // right motor group
                              12, // 12 inch track width
                              lemlib::Omniwheel::NEW_275, // using new 2.75" omnis
                              450, // drivetrain rpm is 450
                              2 // horizontal drift is 2 (for now)
);

// input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttle_curve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain  *1.019   #A
);

// input curve for steer input during driver control
lemlib::ExpoDriveCurve steer_curve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain *1.019   #A
);






//Sensors
pros::Imu imu(18);
pros::Rotation horizontal_rotation_sensor(-20);
pros::Rotation vertical_rotation_sensor(19);
pros::Optical color_sensor(1);

// horizontal tracking wheel
lemlib::TrackingWheel horizontal_tracking_wheel(&horizontal_rotation_sensor, lemlib::Omniwheel::NEW_2, -2.74);
// vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&vertical_rotation_sensor, lemlib::Omniwheel::NEW_2, 0.5);

lemlib::OdomSensors sensors(&vertical_tracking_wheel, // vertical tracking wheel 1, set to null
							nullptr,
                            &horizontal_tracking_wheel, // horizontal tracking wheel 1
							nullptr,
                            &imu // inertial sensor
);


// lateral PID controller
lemlib::ControllerSettings lateral_controller(11, // proportional gain (kP)  10
                                              0, // integral gain (kI)  0
                                              3, // derivative gain (kD)  3
                                              3, // anti windup  3
                                              1, // small error range, in inches  1
                                              100, // small error range timeout, in milliseconds  100
                                              3, // large error range, in inches  3
                                              500, // large error range timeout, in milliseconds    500
                                              20 // maximum acceleration (slew)   20
);

// angular PID controller
lemlib::ControllerSettings angular_controller(4, // proportional gain (kP)   2
                                              0, // integral gain (kI)   0
                                              10, // derivative gain (kD)   10
                                              3, // anti windup    3
                                              1, // small error range, in degrees  1
                                              100, // small error range timeout, in milliseconds  100
                                              3, // large error range, in degrees  3
                                              500, // large error range timeout, in milliseconds  500
                                              0 // maximum acceleration (slew)  0
);


lemlib::Chassis chassis(Drivetrain,
                        lateral_controller,
                        angular_controller,
                        sensors,
                        &throttle_curve, 
                        &steer_curve
);


ASSET(leftfirst_txt);
ASSET(leftsecond_txt);


void left_side_auto() {
    ASSET(leftfirst_txt);
    ASSET(leftsecond_txt);
    chassis.setPose(-62.168, 16.055, 0);
	stage1(127);
	chassis.follow(leftfirst_txt, 15, 3000);
	chassis.moveToPose(-7.864, 9, 316, 3000, {.forwards = false});
	redirect.set_value(false);
	chassis.waitUntil(27);
	stage2(127); 
	chassis.waitUntil(1);
	scraper.set_value(true);
	redirect.set_value(true);
	chassis.follow(leftsecond_txt, 15, 5000);
	stage2(0);
	stage1(127);
	chassis.moveToPose(-26.882, 47.101, 270, 2000, {.forwards = false});
	stage2(127);
	chassis.moveToPose(-62.503, 46.818, 270, 2000);
}