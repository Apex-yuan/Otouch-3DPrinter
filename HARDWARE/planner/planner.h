#ifndef planner_h
#define planner_h

#include "marlin_main.h"


#define FLASH_SET_STORE_OFFSET                5*1024*1024
#define FLASH_SET_EEPROM_VERSION "V01"

#define AXIS_STEPS_PER_UNIT_X_ADDRESS        FLASH_SET_STORE_OFFSET+4		//104-107
#define AXIS_STEPS_PER_UNIT_Y_ADDRESS        FLASH_SET_STORE_OFFSET+8		//108-111
#define AXIS_STEPS_PER_UNIT_Z_ADDRESS        FLASH_SET_STORE_OFFSET+12	//112-115
#define AXIS_STEPS_PER_UNIT_E0_ADDRESS       FLASH_SET_STORE_OFFSET+16	//116-119
#define MAX_FEEDRATE_X_ADDRESS            	 FLASH_SET_STORE_OFFSET+20	//120-123
#define MAX_FEEDRATE_Y_ADDRESS        			 FLASH_SET_STORE_OFFSET+24	//124-127
#define MAX_FEEDRATE_Z_ADDRESS        			 FLASH_SET_STORE_OFFSET+28	//128-131
#define MAX_FEEDRATE_E0_ADDRESS        			 FLASH_SET_STORE_OFFSET+32	//132-135



//#define DEBUG_PLANNER


extern unsigned long minsegmenttime;
extern float max_feedrate[4]; // set the max speeds
extern float axis_steps_per_unit[4];
extern unsigned long max_acceleration_units_per_sq_second[4]; // Use M201 to override by software
extern float minimumfeedrate;
extern float acceleration;         // Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
extern float retract_acceleration; //  mm/s^2   filament pull-pack and push-forward  while standing still in the other axis M204 TXXXX
extern float max_xy_jerk; //speed than can be stopped at once, if i understand correctly.
extern float max_z_jerk;
extern float max_e_jerk;
extern float mintravelfeedrate;
extern unsigned long axis_steps_per_sqr_second[NUM_AXIS];



#ifdef AUTOTEMP
    extern bool autotemp_enabled;
    extern float autotemp_max;
    extern float autotemp_min;
    extern float autotemp_factor;
#endif
// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in 
// the source g-code and may never actually be reached if acceleration management is active.
typedef struct {
  // Fields used by the bresenham algorithm for tracing the line
  long steps_x, steps_y, steps_z, steps_e;  //ÿ�������������ߵĲ��� Step count along each axis
  unsigned long step_event_count;           //������block�����ߵĲ���������4�������ֵ The number of step events required to complete this block
  long accelerate_until;                    //���������еļ��پ��� The index of the step event on which to stop acceleration
  long decelerate_after;                    //���ٺ����ٵľ��� The index of the step event on which to start decelerating
  long acceleration_rate;                   //�����ʣ�����������ٶ� The acceleration rate used for acceleration calculation
  unsigned char direction_bits;             //���block�ķ���λ��1����0����ÿһ��λ����һ����ķ��� The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)
  unsigned char active_extruder;            //�����õ�����Ч����ͷ Selects the active extruder
  #ifdef ADVANCE
    long advance_rate;
    volatile long initial_advance;
    volatile long final_advance;
    float advance;
  #endif

  // Fields used by the motion planner to manage acceleration
//  float speed_x, speed_y, speed_z, speed_e;        // Nominal mm/sec for each axis
  float nominal_speed;                               //��ٶȣ����������ߵ����ٽ׶��ٶ� The nominal speed for this block in mm/sec 
  float entry_speed;                                 //�����ٶȣ�������һ��block���뵽���blockʱ���ٶ� Entry speed at previous-current junction in mm/sec
  float max_entry_speed;                             //�������ٶȣ������ٶȲ��ܳ������ֵ Maximum allowable junction entry speed in mm/sec
  float millimeters;                                 //��·�̣���λmm The total travel of this block in mm
  float acceleration;                                //���ٶȣ���λmm/sec^2    acceleration mm/sec^2
  unsigned char recalculate_flag;                    //���Ӵ����¼��������ٶ����ߵı�־ Planner flag to recalculate trapezoids on entry junction
  unsigned char nominal_length_flag;                 //�ܴﵽ��ٶȵı�־ Planner flag for nominal speed always reached

  // Settings for the trapezoid generator     //�����ٶ����߲����������ò���
  unsigned long nominal_rate;                        //���block�Ķ�ٶ�steps/sec         The nominal step rate for this block in step_events/sec 
  unsigned long initial_rate;                        //�������ߵĳ�ʼ�ٶ�/�����ٶ� The jerk-adjusted step rate at start of block  
  unsigned long final_rate;                          //�������ߵ��˳��ٶ� The minimal rate at exit
  unsigned long acceleration_st;                     //���ٶ� acceleration steps/sec^2
  unsigned long fan_speed;                           //�����ٶ�
  #ifdef BARICUDA
  unsigned long valve_pressure;
  unsigned long e_to_p_pressure;
  #endif
  volatile char busy;//���ڴ������block�ı�־λ��1��ʾ����ִ�����block
} block_t;

void plan_init(void);
// Add a new linear movement to the buffer. x, y and z is the signed, absolute target position in 
// millimaters. Feed rate specifies the speed of the motion.
void plan_buffer_line(const float x, const float y, const float z, const float e, float feed_rate, const uint8_t extruder);

// Set position. Used for G92 instructions. //�趨λ�ã�����G92ָ��
void plan_set_position(const float x, const float y, const float z, const float e);
void plan_set_e_position(const float e);

void check_axes_activity(void); 
uint8_t movesplanned(void); //return the nr of buffered moves

extern float axis_steps_per_unit[4];
// Called when the current block is no longer needed. Discards the block and makes the memory
// availible for new blocks.    
void plan_discard_current_block(void);
block_t *plan_get_current_block(void);
bool blocks_queued(void);

void allow_cold_extrudes(bool allow);
void reset_acceleration_rates(void);

#endif
