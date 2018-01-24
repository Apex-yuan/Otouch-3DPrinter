#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "sys.h"

// This configurstion file contains the basic settings.
// Advanced settings can be found in Configuration_adv.h
// BASIC SETTINGS: select your board type, temperature sensor type, axis scaling, and endstop configuration
//�������ð������������ͣ��¶ȴ��������ͣ������ã���λ��������

//User specified version info of this build to display in [Pronterface, etc] terminal window during startup.
//Implementation of an idea by Prof Braino to inform user that any changes made
//to this build by the user have been successfully uploaded into firmware.
#define STRING_VERSION_CONFIG_H __DATE__ " " __TIME__ // build date and time
#define STRING_CONFIG_H_AUTHOR "(none, default config)" //Who made the changes.


 //This determines the communication speed of the printer//����������
#define BAUDRATE 115200

//// The following define selects which electronics board you have. Please choose the one that matches your setup
//��������ѡ��Դ���չ̼��е���������ѡ����Ҫ��Ϊ�����ò�ͬ����������ŷ��䡣

#ifndef MOTHERBOARD
//#define MOTHERBOARD 7
#endif

// This defines the number of extruders //������ͷ����Ŀ
#define EXTRUDERS 1

//// The following define selects which power supply you have. Please choose the one that matches your setup
// 1 = ATX
// 2 = X-Box 360 203Watts (the blue wire connected to PS_ON and the red wire to VCC)

#define POWER_SUPPLY 1

//===========================================================================
//=============================Thermal Settings  ============================
//===========================================================================
//
//--NORMAL IS 4.7kohm PULLUP!-- 1kohm pullup can be used on hotend sensor, using correct resistor and table
//
//// Temperature sensor settings:
// -2 is thermocouple with MAX6675 (only for sensor 0)
// -1 is thermocouple with AD595
// 0 is not used
// 1 is 100k thermistor - best choice for EPCOS 100k (4.7k pullup)//
// 2 is 200k thermistor - ATC Semitec 204GT-2 (4.7k pullup)
// 3 is mendel-parts thermistor (4.7k pullup)
// 4 is 10k thermistor !! do not use it for a hotend. It gives bad resolution at high temp. !!
// 5 is 100K thermistor - ATC Semitec 104GT-2 (Used in ParCan) (4.7k pullup)
// 6 is 100k EPCOS - Not as accurate as table 1 (created using a fluke thermocouple) (4.7k pullup)
// 7 is 100k Honeywell thermistor 135-104LAG-J01 (4.7k pullup)
// 8 is 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)
// 9 is 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
// 10 is 100k RS thermistor 198-961 (4.7k pullup)
//
//    1k ohm pullup tables - This is not normal, you would have to have changed out your 4.7k for 1k
//                          (but gives greater accuracy and more stable PID)
// 51 is 100k thermistor - EPCOS (1k pullup)
// 52 is 200k thermistor - ATC Semitec 204GT-2 (1k pullup)
// 55 is 100k thermistor - ATC Semitec 104GT-2 (Used in ParCan) (1k pullup)


#define TEMP_SENSOR_0 1//   1����ѡ��100k��NTC����������Ϊ������                                                                 
#define TEMP_SENSOR_1 0
#define TEMP_SENSOR_2 0
#define TEMP_SENSOR_BED 1

// Actual temperature must be close to target for this long before M109 returns success
#define TEMP_RESIDENCY_TIME 10  // (seconds)
#define TEMP_HYSTERESIS 3       // (degC) range of +/- temperatures considered "close" to the target one
//���ֵ������ʵ���¶�������3���Ǳ���Ϊ�ӽ��趨��Ŀ���¶�ֵ���ʵ��Ӵ��ֵ���Լ��ٵȴ����µ�ʱ�䣬������Գ�˿�����������ֵ����Ĭ��
#define TEMP_WINDOW     1       // (degC) Window around target to start the recidency timer x degC early.

// The minimal temperature defines the temperature below which the heater will not be enabled It is used
// to check that the wiring to the thermistor is not broken.
// Otherwise this would lead to the heater being powered on all the time.
#define HEATER_0_MINTEMP 5                                                                                                        
#define HEATER_1_MINTEMP 5
#define HEATER_2_MINTEMP 5
#define BED_MINTEMP 5
//����������ȴ�������¶ȣ����ڸ��¶�ʱ����ӡ������������������Ϊ�������Ҽ���ͷ���ȴ��ļ����޷���

// When temperature exceeds max temp, your heater will be switched off.
// This feature exists to protect your hotend from overheating accidentally, but *NOT* from thermistor short/failure!
// You should use MINTEMP for thermistor short/failure protection.
#define HEATER_0_MAXTEMP 275                                                                                    
#define HEATER_1_MAXTEMP 275
#define HEATER_2_MAXTEMP 275
#define BED_MAXTEMP 150                                                                                                 
//������������ȴ�������¶����ã���ֹ�ջ�����

// If your bed has low resistance e.g. .6 ohm and throws the fuse you can duty cycle it to reduce the
// average current. The value should be an integer and the heat bed will be turned on for 1 interval of
// HEATER_BED_DUTY_CYCLE_DIVIDER intervals.
//#define HEATER_BED_DUTY_CYCLE_DIVIDER 4
//�������������Ϊ�˷�ֹ�ȴ�����̫С����ʱ����������ջ�mos�ܣ�����������֣����Է�ֹmos�ܹ��ȣ�������ʱ�������

//�����ǹ���PID���ڵĲ���                                                                                                               
//PID�¿����ã�PID�������ã���Ҫ����ϵͳ������ã�����ͨ��M303�������PID_autotune������û���PID������
//Ȼ������޸������DEFAULT_Kp��DEFAULT_Ki��DEFAULT_Kd�����磺M303 E0 C8 S190����ʾ��ȡ��ӡͷ0��Ŀ���¶�
//����Ϊ190�ȡ�ѭ������PID_autotune  8�ź����Ӧ��PID������ϵͳ�Զ����ɵ�PID�����൱�����ɱ���Ĭ�ϡ�
//��Ȼ������ͬ��Ҳ��һ������Ч

// PID settings:    
// Comment the following line to disable PID and enable bang-bang.
#define PIDTEMP
#define BANG_MAX 255 // limits current to nozzle while in bang-bang mode; 255=full current
#define PID_MAX 255 // limits current to nozzle while PID is active (see PID_FUNCTIONAL_RANGE below); 255=full current
#ifdef PIDTEMP
//#define PID_DEBUG // Sends debug data to the serial port.
  //#define PID_OPENLOOP 1 // Puts PID in open loop. M104/M140 sets the output power from 0 to PID_MAX
  #define PID_FUNCTIONAL_RANGE 10 // If the temperature difference between the target temperature and the actual temperature
                                  // is more then PID_FUNCTIONAL_RANGE then the PID will be shut off and the heater will be set to min/max.
  #define PID_INTEGRAL_DRIVE_MAX 255  //limit for the integral term
  #define K1 0.95 //smoothing factor withing the PID
  #define PID_dT ((OVERSAMPLENR * 4.0)/1000) //sampling period of the temperature routine


// If you are using a preconfigured hotend then you can use one of the value sets by uncommenting it
// Ultimaker
    #define  DEFAULT_Kp 22.2
    #define  DEFAULT_Ki 1.08
    #define  DEFAULT_Kd 144

// Makergear
//    #define  DEFAULT_Kp 7.0
//    #define  DEFAULT_Ki 0.1
//    #define  DEFAULT_Kd 12

// Mendel Parts V9 on 12V
//    #define  DEFAULT_Kp 63.0
//    #define  DEFAULT_Ki 2.25
//    #define  DEFAULT_Kd 440
#endif // PIDTEMP

// Bed Temperature Control
// Select PID or bang-bang with PIDTEMPBED.  If bang-bang, BED_LIMIT_SWITCHING will enable hysteresis
//
// uncomment this to enable PID on the bed.   It uses the same frequency PWM as the extruder.
// If your PID_dT above is the default, and correct for your hardware/configuration, that means 7.689Hz,
// which is fine for driving a square wave into a resistive load and does not significantly impact you FET heating.
// This also works fine on a Fotek SSR-10DA Solid State Relay into a 250W heater.
// If your configuration is significantly different than this and you don't understand the issues involved, you proabaly
// shouldn't use bed PID until someone else verifies your hardware works.
// If this is enabled, find your own PID constants below.
//#define PIDTEMPBED
//
//#define BED_LIMIT_SWITCHING

// This sets the max power delived to the bed, and replaces the HEATER_BED_DUTY_CYCLE_DIVIDER option.
// all forms of bed control obey this (PID, bang-bang, bang-bang with hysteresis)
// setting this to anything other than 256 enables a form of PWM to the bed just like HEATER_BED_DUTY_CYCLE_DIVIDER did,
// so you shouldn't use it unless you are OK with PWM on your bed.  (see the comment on enabling PIDTEMPBED)
#define MAX_BED_POWER 255 // limits duty cycle to bed; 255=full current

#ifdef PIDTEMPBED
//120v 250W silicone heater into 4mm borosilicate (MendelMax 1.5+)
//from FOPDT model - kp=.39 Tp=405 Tdead=66, Tc set to 79.2, argressive factor of .15 (vs .1, 1, 10)
    #define  DEFAULT_bedKp 10.00
    #define  DEFAULT_bedKi .023
    #define  DEFAULT_bedKd 305.4

//120v 250W silicone heater into 4mm borosilicate (MendelMax 1.5+)
//from pidautotune
//    #define  DEFAULT_bedKp 97.1
//    #define  DEFAULT_bedKi 1.41
//    #define  DEFAULT_bedKd 1675.16

// FIND YOUR OWN: "M303 E-1 C8 S90" to run autotune on the bed at 90 degreesC for 8 cycles.
#endif // PIDTEMPBED



//this prevents dangerous Extruder moves, i.e. if the temperature is under the limit
//can be software-disabled for whatever purposes by
#define PREVENT_DANGEROUS_EXTRUDE
//if PREVENT_DANGEROUS_EXTRUDE is on, you can still disable (uncomment) very long bits of extrusion separately.
#define PREVENT_LENGTHY_EXTRUDE

#define EXTRUDE_MINTEMP 5 //170//��ֵ��ֹ����ͷ�¶�δ�ﵽ�趨Ŀ���¶ȶ����м�������ʱ��Ǳ�ڷ���

//�����ֵ���Ƽ�������󳤶ȣ������ó��ȣ�������������
#define EXTRUDE_MAXLENGTH (X_MAX_LENGTH+Y_MAX_LENGTH) //prevent extrusion of very large distances.


//===========================================================================
//=============================Mechanical Settings===========================
//===========================================================================
//��е���ÿ�ʼ

// Uncomment the following line to enable CoreXY kinematics
// #define COREXY

// corse Endstop Settings
#define ENDSTOPPULLUPS // Comment this out (using // at the start of the line) to disable the endstop pullup resistors
//��λ���������������ã������ʹ�õ��ǻ�еʽ����λ���أ��뱣���˲���

//��еʽ��λ�����뱣�ָô�����
#ifndef ENDSTOPPULLUPS//û��������λ������������ʱ����λ������������ϸ�ֿ���
  // fine Enstop settings: Individual Pullups. will be ignord if ENDSTOPPULLUPS is defined
//  #define ENDSTOPPULLUP_XMAX
//  #define ENDSTOPPULLUP_YMAX
//  #define ENDSTOPPULLUP_ZMAX
//  #define ENDSTOPPULLUP_XMIN
//  #define ENDSTOPPULLUP_YMIN
//  #define ENDSTOPPULLUP_ZMIN
#endif

#ifdef ENDSTOPPULLUPS//������λ������������ʱ����λ������������ϸ�ֿ���
  #define ENDSTOPPULLUP_XMAX
  #define ENDSTOPPULLUP_YMAX
  #define ENDSTOPPULLUP_ZMAX
  #define ENDSTOPPULLUP_XMIN
  #define ENDSTOPPULLUP_YMIN
  #define ENDSTOPPULLUP_ZMIN
#endif

//������λ���ش򿪣��Ѿ����ź�תΪ�����źŵ���Ƭ��                                                                            
// The pullups are needed if you directly connect a mechanical endswitch between the signal and ground pins.
////��֪��Ϊʲô������const�����壬�����䶨��ᱨ�� //����һ����Ҫ��.h�ļ��ж���
//const bool X_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
//const bool Y_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
//#ifndef Z_MIN_ENDSTOP_INVERTING
//extern bool const Z_MIN_ENDSTOP_INVERTING;// = true; // set to true to invert the logic of the endstop.
//#endif
//const bool X_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
//const bool Y_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
//const bool Z_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
#define X_MIN_ENDSTOP_INVERTING true // set to true to invert the logic of the endstop.
#define Y_MIN_ENDSTOP_INVERTING true // set to true to invert the logic of the endstop.
#define Z_MIN_ENDSTOP_INVERTING true // set to true to invert the logic of the endstop.
#define X_MAX_ENDSTOP_INVERTING true // set to true to invert the logic of the endstop.
#define Y_MAX_ENDSTOP_INVERTING true // set to true to invert the logic of the endstop.
#define Z_MAX_ENDSTOP_INVERTING true // set to true to invert the logic of the endstop.
//#define DISABLE_MAX_ENDSTOPS
//#define DISABLE_MIN_ENDSTOPS

// For Inverting Stepper Enable Pins (Active Low) use 0, Non Inverting (Active High) use 1                                    
#define X_ENABLE_ON 0  //�������ʹ��״̬���͵�ƽ������״̬ʹ�ܣ��ߵ�ƽΪ�ѻ�״̬
#define Y_ENABLE_ON 0
#define Z_ENABLE_ON 0
#define E_ENABLE_ON 0 // For all extruders

//ͨ�����������Ĵ����ǲ��Ķ��ģ������ᶼ��ѡ��false�ġ�Ȼ����������3d��ӡ��z�����ֶ������Ĳ�����
//������#define DISABLE_Z �и�Ϊtrue �������ڴ�ӡ����ӡʱ�������ֶ�����z��
// Disables axis when it's not being used. ���ĸ��᲻�˶�ʱ�Ƿ�رյ����
#define DISABLE_X 0
#define DISABLE_Y 0
#define DISABLE_Z 0
#define DISABLE_E 0 // For all extruders

//�⼸�������ǱȽ����״�ġ������Լ���е�����Ͳ�ͬ�����������ò�����ͬ������ԭ�����Ҫ��֤
//ԭ��Ӧ���ڴ�ӡƽ̨�����½ǣ�ԭ��λ��Ϊ[0,0]���������Ͻǣ�ԭ��λ��Ϊ[max,max]����
//ֻ��������ӡ������ģ�Ͳ�����ȷ�ģ��������ĳ����ľ�������ģ�ͷ�λ���ԡ�
//�޸Ķ�Ӧĳ��������ã�true��false���󣬵���ᷴ��
#define INVERT_X_DIR  false //true//false    // for Mendel set to false, for Orca set to true                                     
#define INVERT_Y_DIR  true //true//false    // for Mendel set to true, for Orca set to false
#define INVERT_Z_DIR  false //true     // for Mendel set to false, for O      rca set to true
#define INVERT_E0_DIR false //true    // for direct drive extruder v9 set to true, for geared extruder set to false
#define INVERT_E1_DIR false    // for direct drive extruder v9 set to true, for geared extruder set to false
//#define INVERT_E2_DIR false    // for direct drive extruder v9 set to true, for geared extruder set to false

//��ԭ�㷽�����á����ԭ��λ��Ϊ��Сֵ����Ϊ-1�����ԭ��λ��Ϊ���ֵ����Ϊ1
// ENDSTOP SETTINGS:                                                                                           
// Sets direction of endstops when homing; 1=MAX, -1=MIN
#define X_HOME_DIR -1
#define Y_HOME_DIR -1
#define Z_HOME_DIR -1

#define min_software_endstops true //If true, axis won't move to coordinates less than HOME_POS.
#define max_software_endstops true  //If true, axis won't move to coordinates greater than the defined lengths below.

//�⼸�����������ô�ӡ�ߴ����Ҫ������������Ҫ˵����������ԭ�㲢���Ǵ�ӡ���ģ�
//�����Ĵ�ӡ����һ����[[(x.max-x.min)/2,(y.max-y.min)/2]]��λ�á�
//����λ��������Ҫ�ں������Ƭ������ʹ�õ�����ӡ��������Ӧ��������Ĳ�������ƥ�䣬����ܿ��ܻ��ӡ��ƽ̨����
// Travel limits after homing
#define X_MAX_POS 200                                                                                              
#define X_MIN_POS 0 //-250 //0
#define Y_MAX_POS 150 //0 //150
#define Y_MIN_POS 0 //-150 //0
#define Z_MAX_POS 150
#define Z_MIN_POS 0

#define X_MAX_LENGTH (X_MAX_POS - X_MIN_POS)
#define Y_MAX_LENGTH (Y_MAX_POS - Y_MIN_POS)
#define Z_MAX_LENGTH (Z_MAX_POS - Z_MIN_POS)

// The position of the homing switches
//��λ��������
//#define MANUAL_HOME_POSITIONS  // If defined, MANUAL_*_HOME_POS below will be used
//������������ã����� MANUAL_*_HOME_POS���ý���Ч
//#define BED_CENTER_AT_0_0  // If defined, the center of the bed is at (X=0, Y=0)
////ȡ������ע�ͣ��ɽ��ȴ����Ķ���ΪX=0��Y=0

//�ֶ����û�ԭ��λ�ã����Ҫʹ�øù��ܣ��뽫����#define MANUAL_HOME_POSITIONSǰ�ġ�//��ɾ����
//ʹ�øù��ܺ�Ĭ�ϻ�ԭ��λ�ý��������趨����������ֵ
//Manual homing switch locations:
//��������������ζ�ŵѿ�����ӡ���Ķ��������ĵ�ֵ��
#define MANUAL_X_HOME_POS 0 //(X_MAX_POS - X_MIN_POS)/2 //0
#define MANUAL_Y_HOME_POS 0 //(Y_MAX_POS - Y_MIN_POS)/2 //0
#define MANUAL_Z_HOME_POS 0

//// MOVEMENT SETTINGS//������                                                                                                         
#define NUM_AXIS 4 // The axis order in all axis related arrays is X, Y, Z, E
//��������������������˳����X, Y, Z, E
#define HOMING_FEEDRATE {50*60, 50*60, 10*60, 0}  // set the homing speeds (mm/min)                                             
//��������Ϊ��ԭ����ٶȣ��ɸ���ʵ���������Ӧ��������λ��mm/min
// default settings

#define DEFAULT_AXIS_STEPS_PER_UNIT   {94.1176,94.1176,400.0000,97.0087} //{92,92,400,158.8308}  // default steps per unit for ultimaker                         
//��������Ǵ�ӡ����ӡ�ߴ��Ƿ���ȷ������Ҫ��������������Ϊ��������1mm����Ҫ�����������ֱ��Ӧx,y,z,e���ᡣ
//���������������ֶ���Ҫ�Լ�����ſ��ԡ����ɲο�http://calculator.josefprusa.cz/��
//ͬ��������ʱ�ļ��㹫ʽ��X\Y�ᣩ���������ÿת����(1.8�Ȳ���ǵĵ��Ϊ200,0.9�Ȳ���ǵĵ��Ϊ400)*�����������ϸ�����ã�һ��16ϸ�֣�/ͬ�����ݼ��/ͬ���ֳ���
//˿�ܴ���ʱ�ļ��㹫ʽ��Z�ᣩ���������ÿת����*�����������ϸ������/˿�ܵ���
//���������㹫ʽ��E�ᣩ���������ÿת����*�����������ϸ������*���������ִ�����/�������ܳ�


#define DEFAULT_MAX_FEEDRATE          {500, 500, 5, 25}    // (mm/sec)                                                                                                             
//���ø����������ٶȡ����ߵ�ֵ��Ҫ����ĵ���������⽫���µ�����ȣ������п���ʹ����ڴ�ӡʱʧ����һ�������Ϊ200-400

                                                                                                                                
#define DEFAULT_MAX_ACCELERATION      {9000,9000,100,10000}  //{3000,3000,100,3000} //100    // X, Y, Z, E maximum start speed for accelerated moves. E default values are good for skeinforge 40+, for older versions raise them a lot.
//������Ϊ��������ٶȡ����ߵĵ�����ٶȽ����µ���ڴ�ӡ����ʱ���壬�Ӷ���ʧ�����齫X\Y�����ٶ��޸�Ϊ1000-3000

//Ĭ�ϴ�ӡ���ٶ�                                                                                                                
#define DEFAULT_ACCELERATION          3000
#define DEFAULT_RETRACT_ACCELERATION  3000   // X, Y, Z and E max acceleration in mm/s^2 for r retracts                         

// Offset of the extruders (uncomment if using more than one and relying on firmware to position when changing).
// The offset has to be X=0, Y=0 for the extruder 0 hotend (default extruder).
// For the other hotends it is their distance from the extruder 0 hotend.
// #define EXTRUDER_OFFSET_X {0.0, 20.00} // (in mm) for each extruder, offset of the hotend on the X axis
// #define EXTRUDER_OFFSET_Y {0.0, 5.00}  // (in mm) for each extruder, offset of the hotend on the Y axis

//������Ϊ���ٶȱ仯�ʡ���ֵ����ͬ���ᵼ�µ����ʧ������ֵ���ں���Χ�ڵĽ�Сֵʱ����ӡ��������ƽ����
//��ӡ����еӦ������С�������ڻ���ʱ���и��õĸ���������ӡ����Ҳ�����ͣ�����ֵ���ں���Χ�ڵĽϴ�ֵ��
//��ӡʱ�佫���̡���������޸ĸ�ֵ����ò���
// The speed change that does not require acceleration (i.e. the software might assume it can be done instanteneously)
//���᲻��Ҫ���ٵľ��룬��������٣�������ɵľ��루�������Ϊ��������˲����ɵģ�
#define DEFAULT_XYJERK                 20.0       // (mm/sec)
#define DEFAULT_ZJERK                 0.4     // (mm/sec)
#define DEFAULT_EJERK                 5.0    // (mm/sec)

//===========================================================================
//=============================Additional Features===========================
//===========================================================================

// EEPROM
// the microcontroller can store settings in the EEPROM, e.g. max velocity...
// M500 - stores paramters in EEPROM
// M501 - reads parameters from EEPROM (if you need reset them after you changed them temporarily).
// M502 - reverts to the default "factory settings".  You still need to store them in EEPROM afterwards if you want to.
//define this to enable eeprom support
//�Ƿ���EEPROM�������󣬿���ͨ��Gcode��LCD���޸ģ����룬������ز�����
//#define EEPROM_SETTINGS
//to disable EEPROM Serial responses and decrease program space by ~1700 byte: comment this out:
// please keep turned on if you can.
//�رմ����޸�EEPROM�Ĺ���
//#define EEPROM_CHITCHAT

// Preheat Constants //Ԥ�Ȳ������ã��������޸�
#define PLA_PREHEAT_HOTEND_TEMP 180 
#define PLA_PREHEAT_HPB_TEMP 50
#define PLA_PREHEAT_FAN_SPEED 255   // Insert Value between 0 and 255

#define ABS_PREHEAT_HOTEND_TEMP 240
#define ABS_PREHEAT_HPB_TEMP 80
#define ABS_PREHEAT_FAN_SPEED 255   // Insert Value between 0 and 255

//LCD and SD support//LCD��SD�����ã���������LCD��������Ӧ���ã��ٴβ���˵��
//#define ULTRA_LCD  //general lcd support, also 16x2
//#define DOGLCD  // Support for SPI LCD 128x64 (Controller ST7565R graphic Display Family)
#define SDSUPPORT // Enable SD Card Support in Hardware Console
//#define SDSLOW // Use slower SD transfer mode (not normally needed - uncomment if you're getting volume init error)


// Increase the FAN pwm frequency. Removes the PWM noise but increases heating in the FET/Arduino
//#define FAST_PWM_FAN

// Use software PWM to drive the fan, as for the heaters. This uses a very low frequency
// which is not ass annoying as with the hardware PWM. On the other hand, if this frequency
// is too low, you should also increment SOFT_PWM_SCALE.
#define FAN_SOFT_PWM  //ĿǰӲ��δ��Ӳ��PWM��֧�֣������Ȳ������PWM��2017/4/16ע�� 

// Incrementing this by 1 will double the software PWM frequency,
// affecting heaters, and the fan if FAN_SOFT_PWM is enabled.
// However, control resolution will be halved for each increment;
// at zero value, there are 128 effective control positions.
#define SOFT_PWM_SCALE 0

// M240  Triggers a camera by emulating a Canon RC-1 Remote
// Data from: http://www.doc-diy.net/photo/rc-1_hacked/
// #define PHOTOGRAPH_PIN     23

// SF send wrong arc g-codes when using Arc Point as fillet procedure
//#define SF_ARC_FIX

// Support for the BariCUDA Paste Extruder.
//#define BARICUDA

/*********************************************************************\
* R/C SERVO support
* Sponsored by TrinityLabs, Reworked by codexmas
**********************************************************************/

// Number of servos
//
// If you select a configuration below, this will receive a default value and does not need to be set manually
// set it manually if you have more servos than extruders and wish to manually control some
// leaving it undefined or defining as 0 will disable the servo subsystem
// If unsure, leave commented / disabled
//
// #define NUM_SERVOS 3

#include "Configuration_adv.h"

#endif //__CONFIGURATION_H
