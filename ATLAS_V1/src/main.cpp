#include "lemlib/api.hpp" // IWYU pragma: keep
#include "main.h"
#include "liblvgl/display/lv_display.h"
#include "liblvgl/misc/lv_area.h"
#include "liblvgl/widgets/image/lv_image.h"

//Controller
pros::Controller master(pros::E_CONTROLLER_MASTER);

//Drivetrain motorgroups
pros::MotorGroup left_mg({-13, -6, 5}, pros::MotorGearset::blue);
pros::MotorGroup right_mg({14, 11, -12}, pros::MotorGearset::blue);
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



//Pneumatics
pros::adi::Pneumatics descore({17, 'a'}, false);   
pros::adi::Pneumatics park({17, 'b'}, false);  
pros::adi::Pneumatics scraper({17, 'c'}, false);  
pros::adi::Pneumatics redirect({17, 'd'}, false);  




LV_IMAGE_DECLARE(VEX_screensaverv3);
void display_img_from_c_array() {

	lv_obj_t*img1 = lv_image_create(lv_screen_active());
	lv_image_set_src(img1, &VEX_screensaverv3);
	lv_obj_align(img1,LV_ALIGN_TOP_LEFT, 0, -10);



}

void display_img_from_file(const void * src){

}

void initialize() {
	display_img_from_c_array();
	chassis.calibrate();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
ASSET(leftfirst_txt);

void autonomous() {
// set position to x:0, y:0, heading:0
	chassis.setPose(-62.549, 16.342, 0);
	stage1(127);
	chassis.follow(leftfirst_txt, 15, 2000);
	chassis.waitUntil(50);
	stage1(0);

	
}
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */

void opcontrol() {
	
	
	while (true) {
	
        int power = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y); 
        int turn = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);


        int left = power + turn;
        int right = power - turn;
		
 
	 	chassis.arcade(power, turn, false, 0.65); // #A     change num


		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
			stage1(127);
		}

		else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
			stage1(-127);
		}

		else{
			stage1(0);
		}

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
			stage2(127);
		}

		else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
			stage2(-127);
		}

		else{
			stage2(0);
		}

		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)) {
       		descore.toggle();
		}

		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)) {
       		park.toggle();
		}

		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
       		scraper.toggle();
		}

		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)) {
       		redirect.toggle();
		}

	
        // A small delay is necessary to prevent the brain from overloading
        pros::delay(20); 
	} 	                         // Run for 20 ms then update                        // Run for 20 ms then update
	
}