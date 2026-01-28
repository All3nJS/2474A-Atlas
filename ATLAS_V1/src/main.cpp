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

//Pneumatics
pros::adi::Pneumatics descore({17, 'a'}, false);   
pros::adi::Pneumatics park({17, 'b'}, false);  
pros::adi::Pneumatics scraper({17, 'c'}, false);  
pros::adi::Pneumatics redirect({17, 'd'}, false);  

int currentautonselected = -1;
static lv_obj_t* selectscreen = NULL;
static lv_obj_t* confirmscreen = NULL;
int temporaryselectedauton = 0;

void showConfirmScreen(int autonChoice);
void confirmSelection(lv_event_t * e);
void goBackToSelection(lv_event_t * e);

static void leftBtnClick(lv_event_t * e) {
    temporaryselectedauton = 0;
    showConfirmScreen(0);
}

static void rightBtnClick(lv_event_t * e) {
    temporaryselectedauton = 1;
    showConfirmScreen(1);
}

static void skillsBtnClick(lv_event_t * e) {
    temporaryselectedauton = 2;
    showConfirmScreen(2);
}

void showConfirmScreen(int autonChoice) {
    lv_obj_add_flag(selectscreen, LV_OBJ_FLAG_HIDDEN);
    
    if(confirmscreen != NULL) {
        lv_obj_delete(confirmscreen);
    }
    
    confirmscreen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(confirmscreen, 480, 240);
    lv_obj_center(confirmscreen);
    lv_obj_remove_flag(confirmscreen, LV_OBJ_FLAG_SCROLLABLE);
    
    const char* autonName = (autonChoice == 0) ? "LEFT SIDE" : (autonChoice == 1) ? "RIGHT SIDE" : "SKILLS";
    
    lv_obj_t* confirmTitle = lv_label_create(confirmscreen);
    lv_label_set_text(confirmTitle, autonName);
    lv_obj_align(confirmTitle, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_text_font(confirmTitle, &lv_font_montserrat_24, 0);
    
    lv_obj_t* confirmBtn = lv_button_create(confirmscreen);
    lv_obj_set_size(confirmBtn, 200, 80);
    lv_obj_align(confirmBtn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(confirmBtn, confirmSelection, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(confirmBtn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    
    lv_obj_t* confirmLabel = lv_label_create(confirmBtn);
    lv_label_set_text(confirmLabel, "CONFIRM");
    lv_obj_center(confirmLabel);
    lv_obj_set_style_text_font(confirmLabel, &lv_font_montserrat_20, 0);
    
    lv_obj_t* backBtn = lv_button_create(confirmscreen);
    lv_obj_set_size(backBtn, 120, 50);
    lv_obj_align(backBtn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_event_cb(backBtn, goBackToSelection, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(backBtn, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    
    lv_obj_t* backLabel = lv_label_create(backBtn);
    lv_label_set_text(backLabel, "BACK");
    lv_obj_center(backLabel);
}

void goBackToSelection(lv_event_t * e) {
    if(confirmscreen != NULL) {
        lv_obj_delete(confirmscreen);
        confirmscreen = NULL;
    }
    lv_obj_remove_flag(selectscreen, LV_OBJ_FLAG_HIDDEN);
}

void confirmSelection(lv_event_t * e) {
    currentautonselected = temporaryselectedauton;
    
    lv_obj_delete(confirmscreen);
    confirmscreen = NULL;
    
    lv_obj_t* doneScreen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(doneScreen, 480, 240);
    lv_obj_center(doneScreen);
    lv_obj_remove_flag(doneScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(doneScreen, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    
    const char* autonName = (currentautonselected == 0) ? "LEFT SIDE" : (currentautonselected == 1) ? "RIGHT SIDE" : "SKILLS";
    
    lv_obj_t* doneTitle = lv_label_create(doneScreen);
    lv_label_set_text(doneTitle, "READY");
    lv_obj_align(doneTitle, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_font(doneTitle, &lv_font_montserrat_24, 0);
    
    lv_obj_t* doneLabel = lv_label_create(doneScreen);
    lv_label_set_text(doneLabel, autonName);
    lv_obj_align(doneLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(doneLabel, &lv_font_montserrat_24, 0);
}

LV_IMAGE_DECLARE(VEX_screensaverv3);
void display_img_from_c_array() {
	lv_obj_t*img1 = lv_image_create(lv_screen_active());
	lv_image_set_src(img1, &VEX_screensaverv3);
	lv_obj_align(img1,LV_ALIGN_TOP_LEFT, 0, -10);
}

void menu(void)
{	
	static lv_style_t style;
    lv_style_init(&style);

	static lv_style_t style1;
    lv_style_init(&style1);

    lv_obj_t * menu = lv_menu_create(lv_screen_active());
    lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));
    lv_obj_center(menu);

	lv_obj_t * label;

    lv_obj_t * btn1 = lv_obj_create(lv_screen_active());
    lv_obj_add_style(btn1, &style, 0);
    lv_obj_set_pos(btn1, 158, 180); 
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Red Autos");
    lv_obj_center(label);
	
	lv_obj_set_style_bg_color(btn1, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
	lv_style_set_width(&style, 200);
    lv_style_set_height(&style, LV_SIZE_CONTENT);

	lv_obj_t * btn2 = lv_obj_create(lv_screen_active());
    lv_obj_add_style(btn2, &style, 0);
    lv_obj_set_pos(btn2, 0, 180); 
    lv_obj_remove_flag(btn2, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Blue Autos");
    lv_obj_center(label);
	
	lv_obj_set_style_bg_color(btn2, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
	lv_style_set_width(&style, 200);
    lv_style_set_height(&style, LV_SIZE_CONTENT);

    lv_obj_t * btn4 = lv_obj_create(lv_screen_active());
    lv_obj_add_style(btn4, &style, 0);
    lv_obj_set_pos(btn4, 318, 180); 
    lv_obj_remove_flag(btn4, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn4);
    lv_label_set_text(label, "Skills Autos");
    lv_obj_center(label);
	
	lv_obj_set_style_bg_color(btn4, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
	lv_style_set_width(&style, 162);
    lv_style_set_height(&style, LV_SIZE_CONTENT);
}

void initialize() {
	pros::delay(500);
	
	selectscreen = lv_obj_create(lv_screen_active());
	lv_obj_set_size(selectscreen, 480, 240);
	lv_obj_center(selectscreen);
	lv_obj_remove_flag(selectscreen, LV_OBJ_FLAG_SCROLLABLE);
	
	lv_obj_t* title = lv_label_create(selectscreen);
	lv_label_set_text(title, "AUTONOMOUS SELECTOR");
	lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
	lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

	lv_obj_t* leftBtn = lv_button_create(selectscreen);
	lv_obj_set_size(leftBtn, 140, 60);
	lv_obj_align(leftBtn, LV_ALIGN_CENTER, -160, 0);
	lv_obj_add_event_cb(leftBtn, leftBtnClick, LV_EVENT_CLICKED, NULL);
	lv_obj_set_style_bg_color(leftBtn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
	
	lv_obj_t* leftLabel = lv_label_create(leftBtn);
	lv_label_set_text(leftLabel, "LEFT");
	lv_obj_center(leftLabel);

	lv_obj_t* rightBtn = lv_button_create(selectscreen);
	lv_obj_set_size(rightBtn, 140, 60);
	lv_obj_align(rightBtn, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_event_cb(rightBtn, rightBtnClick, LV_EVENT_CLICKED, NULL);
	lv_obj_set_style_bg_color(rightBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
	
	lv_obj_t* rightLabel = lv_label_create(rightBtn);
	lv_label_set_text(rightLabel, "RIGHT");
	lv_obj_center(rightLabel);

	lv_obj_t* skillsBtn = lv_button_create(selectscreen);
	lv_obj_set_size(skillsBtn, 140, 60);
	lv_obj_align(skillsBtn, LV_ALIGN_CENTER, 160, 0);
	lv_obj_add_event_cb(skillsBtn, skillsBtnClick, LV_EVENT_CLICKED, NULL);
	lv_obj_set_style_bg_color(skillsBtn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
	
	lv_obj_t* skillsLabel = lv_label_create(skillsBtn);
	lv_label_set_text(skillsLabel, "SKILLS");
	lv_obj_center(skillsLabel);
	
	chassis.calibrate();
}

void disabled() {}

void competition_initialize() {}

ASSET(leftfirst_txt);
ASSET(leftsecond_txt);
void autonomous() {
	if (currentautonselected == 0) {
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
	else if (currentautonselected == 1) {

	}
	else if (currentautonselected == 2) {

	}
}

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

        pros::delay(21); 
	}
}