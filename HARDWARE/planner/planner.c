#include "marlin_main.h"
#include "planner.h"
#include "stepper.h"
#include "temperature.h"
//#include "ultralcd.h"
#include "language.h"
//#include <math.h>



//===========================================================================
//=============================public variables ============================
//===========================================================================

unsigned long minsegmenttime; //ִ��·������Сʱ��
float max_feedrate[4]; // set the max speeds //�趨����ٶ�ֵ
float axis_steps_per_unit[4];  //��ÿ�ƶ���λ���ȣ�mm���Ĳ�������������
unsigned long max_acceleration_units_per_sq_second[4]; //����������ٶ�// Use M201 to override by software//ʵ��M201ͨ���������д��
float minimumfeedrate; //����������Сֵ
float acceleration; // ����ٶ�ֵ��mm/s^2 �������������Ĭ�ϼ��ٶ�//Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
float retract_acceleration; //�س���ٶ�  mm/s^2  // filament pull-pack and push-forward  while standing still in the other axis M204 TXXXX
float max_xy_jerk; //�ܹ�����ֹͣ������ٶ� //speed than can be stopped at once, if i understand correctly.
float max_z_jerk;
float max_e_jerk;
float mintravelfeedrate;
unsigned long axis_steps_per_sqr_second[NUM_AXIS]; //

// The current position of the tool in absolute steps
long position[4];   //rescaled from extern when axis_steps_per_unit are changed by gcode //�����λ�����ڼ���·��
static float previous_speed[4]; // Speed of previous path line segment //ǰһ·��������ٶ�
static float previous_nominal_speed; // Nominal speed of previous path line segment //ǰһ·���Ķ�ٶ�

#ifdef AUTOTEMP  //��ִ��G����Ĺ����У�AUTOTEMP����Ƽ���
float autotemp_max=250;
float autotemp_min=210;
float autotemp_factor=0.1;
bool autotemp_enabled=false;
#endif
//===========================================================================
//=================semi-private variables, used in inline  functions    =====
//===========================================================================
block_t block_buffer[BLOCK_BUFFER_SIZE];            // A ring buffer for motion instfructions
volatile unsigned char block_buffer_head;           // Index of the next block to be pushed
volatile unsigned char block_buffer_tail;           // Index of the block to process now

//===========================================================================
//=============================private variables ============================
//===========================================================================
#ifdef PREVENT_DANGEROUS_EXTRUDE
bool allow_cold_extrude=false;
#endif
#ifdef XY_FREQUENCY_LIMIT //���ǻ��
#define MAX_FREQ_TIME (1000000.0/XY_FREQUENCY_LIMIT)
// Used for the frequency limit
static unsigned char old_direction_bits = 0;               // Old direction bits. Used for speed calculations
static long x_segment_time[3]={MAX_FREQ_TIME + 1,0,0};     // Segment times (in us). Used for speed calculations
static long y_segment_time[3]={MAX_FREQ_TIME + 1,0,0};
#endif

// Returns the index of the next block in the ring buffer//�����ڻ��λ������е���һ��block������ֵ
// NOTE: Removed modulo (%) operator, which uses an expensive divide and multiplication.����������仰����
static int8_t next_block_index(int8_t block_index) {
  block_index++;
  if (block_index == BLOCK_BUFFER_SIZE) { 
    block_index = 0; 
  }
  return(block_index);
}


// Returns the index of the previous block in the ring buffer//�����ڻ��λ������е���һ��block������ֵ
static int8_t prev_block_index(int8_t block_index) {
  if (block_index == 0) { 
    block_index = BLOCK_BUFFER_SIZE; 
  }
  block_index--;
  return(block_index);
}

//===========================================================================
//=============================functions         ============================
//===========================================================================

// Calculates the distance (not time) it takes to accelerate from initial_rate to target_rate using the 
// given acceleration://�����Ը����ļ��ٶȴӳ�ʼ�ٶȼ��ٵ�Ŀ���ٶ���Ҫ�ľ���	��������
float estimate_acceleration_distance(float initial_rate, float target_rate, float acceleration)
{
  if (acceleration!=0) {
    return((target_rate*target_rate-initial_rate*initial_rate)/
      (2.0*acceleration));
  }
  else {
    return 0.0;  // acceleration was 0, set acceleration distance to 0//���ٶ�Ϊ0�����趨�ļ��پ���Ϊ0
  }
}

// This function gives you the point at which you must start braking (at the rate of -acceleration) if 
// you started at speed initial_rate and accelerated until this point and want to end at the final_rate after
// a total travel of distance. This can be used to compute the intersection point between acceleration and
// deceleration in the cases where the trapezoid has no plateau (i.e. never reaches maximum speed)
//�����������ʼ�ٶȿ�ʼ���ٵ�����㲢�������ٶȽ��������ó̵ľ��룬������������������ʼɲ���ĵ㣨�Ը��ļ��ٶ����ʣ�
//����Ա�����������ټ��ٵĽ���㣬���������������û�и�ԭ���̵ױߣ�
float intersection_distance(float initial_rate, float final_rate, float acceleration, float distance) 
{
  if (acceleration!=0) {
    return((2.0*acceleration*distance-initial_rate*initial_rate+final_rate*final_rate)/
      (4.0*acceleration) );
  }
  else {
    return 0.0;  // acceleration was 0, set intersection distance to 0
  }
}

// Calculates trapezoid parameters so that the entry- and exit-speed is compensated by the provided factors.
//�������β����Ա�������ٶȱ��ṩ�Ĳ�������
void calculate_trapezoid_for_block(block_t *block, float entry_factor, float exit_factor) 
{
	unsigned long initial_rate = ceil(block->nominal_rate*entry_factor); // (step/min) //ceil():���ش��ڻ��ߵ���ָ�����ʽ����С����
  unsigned long final_rate = ceil(block->nominal_rate*exit_factor); // (step/min)
  unsigned long acceleration = block->acceleration_st;
  #ifdef ADVANCE  //(�ǻ)
  volatile long initial_advance; 
  volatile long final_advance;
  #endif // ADVANCE
  
  int32_t accelerate_steps =   ceil(estimate_acceleration_distance(block->initial_rate, block->nominal_rate,(float)acceleration)); //���μ���б������Ĳ���
  int32_t decelerate_steps =   floor(estimate_acceleration_distance(block->nominal_rate, block->final_rate, -(float)acceleration));//���μ���б������Ĳ���
  // Calculate the size of Plateau of Nominal Rate.//�������ζ�ٶȸ�ԭ���̵ױߣ��Ĵ�С
  int32_t plateau_steps = block->step_event_count-accelerate_steps-decelerate_steps;//�ܲ���-���ٲ���-���ٲ���
	
//	#ifdef DEBUG_PLANNER
//	printf("\n*****calculate_trapezoid_for_block****\n");
//	printf("entry_factor=%f\n",entry_factor);
//	printf("exit_factor=%f\n",exit_factor);
//	printf("block->initial_rate=%ld\n",block->initial_rate);
//	printf("block->final_rate=%ld\n",block->final_rate);
//  printf("block->nominal_rate=%ld\n",block->nominal_rate);
//  printf("acceleration=%ld\n",acceleration);
//  printf("step_event_count=%ld\n",block->step_event_count);
//  printf("accelerate_steps=%d\n",accelerate_steps);
//  printf("decelerate_steps=%d\n",decelerate_steps);
//  printf("plateau_steps=%d\n",plateau_steps);
//	#endif
 
  // Limit minimal step rate (Otherwise the timer will overflow.)//�޶���С�Ĳ������ʣ�����ʱ���������
  if(initial_rate <120) 
  {
    initial_rate=120; 
  }
  if(final_rate < 120) 
  {
    final_rate=120;  
  }
  // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
  // have to use intersection_distance() to calculate when to abort acceleration and start braking
  // in order to reach the final_rate exactly at the end of this block.
	//��ٶȵĲ���С��0������ζ��û��Ѳ������ƽ�ߣ������Ǳ���ʹ��intersection_distance()����������
	//��ʱ��ֹ���������ƶ��Ա������block�Ľ����ﵽfinal_rate��
  if (plateau_steps < 0) {
    accelerate_steps = ceil(intersection_distance(block->initial_rate, block->final_rate, acceleration, block->step_event_count));
    accelerate_steps = max(accelerate_steps,0); // Check limits due to numerical round-off //��������������ֵ���������
    accelerate_steps = min((uint32_t)accelerate_steps,block->step_event_count);//(We can cast here to unsigned, because the above line ensures that we are above zero)
    plateau_steps = 0;
  }

#ifdef ADVANCE //���ǻ��
  initial_advance = block->advance*entry_factor*entry_factor; 
  final_advance = block->advance*exit_factor*exit_factor;
#endif // ADVANCE

  CRITICAL_SECTION_START; //�ؼ����ֿ�ʼ����ֹ���жϴ���ȹر��ж�  
	// Fill variables used by the stepper in a critical section //�������ǲ������ʹ����һ�ؼ��Ĳ���
  if(block->busy == false) { // Don't update variables if block is busy.//block����æ״̬ʱ��Ҫ���±���
    block->accelerate_until = accelerate_steps; //���پ���
    block->decelerate_after = accelerate_steps+plateau_steps; //���ٺ����پ���
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
#ifdef ADVANCE //���ǻ��
    block->initial_advance = initial_advance;
    block->final_advance = final_advance;
#endif //ADVANCE
  }
  CRITICAL_SECTION_END; //�ؼ����ֽ��������ж� 
}                    

//�������������������ٶȣ���������ܹ�����ָ���ľ����ڵļ��ٶȵ���Ŀ���ٶȡ�
// Calculates the maximum allowable speed at this point when you must be able to reach target_velocity using the 
// acceleration within the allotted distance.//���������������ܹ��ڷ���ľ������Լ��ٶȴﵽĿ���ٶȵĵ����Ŀ�������ٶ�ֵ
float max_allowable_speed(float acceleration, float target_velocity, float distance) {  //������ʼ�ٶ�
  return  sqrt(target_velocity*target_velocity-2*acceleration*distance);
}

// "Junction jerk"����ɲ���� in this context is the immediate change in speed at the junction of two blocks.
// ��Junction jerk���������ĵ�������������block�Ľ�ϴ������ı��ٶȡ�
// This method will calculate the junction jerk as the euclidean distance between the nominal 
// velocities of the respective blocks. //����������Լ����ڼ�ɲ���Ը���block�Ķ�ٶȵ�ŷ����¾���
//inline float junction_jerk(block_t *before, block_t *after) {
//  return sqrt(
//    pow((before->speed_x-after->speed_x), 2)+pow((before->speed_y-after->speed_y), 2));
//}


// The kernel���ںˣ� called by planner_recalculate() when scanning the plan from last to first entry.

void planner_reverse_pass_kernel(block_t *previous, block_t *current, block_t *next) {
  if(!current) { 
    return; 
  }

  if (next) {
    // If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
    // If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
    // check for maximum allowable speed reductions to ensure maximum possible planned speed.
    if (current->entry_speed != current->max_entry_speed) {

      // If nominal length true, max junction speed is guaranteed to be reached. Only compute
      // for max allowable speed if block is decelerating and nominal length is false.
      if ((!current->nominal_length_flag) && (current->max_entry_speed > next->entry_speed)) {
        current->entry_speed = min( current->max_entry_speed,
        max_allowable_speed(-current->acceleration,next->entry_speed,current->millimeters));
      } 
      else {
        current->entry_speed = current->max_entry_speed;
      }
      current->recalculate_flag = true;

    }
  } // Skip last block. Already initialized and set for recalculation.
}

// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
// implements the reverse pass.
//planner_recalculate()������Ҫ��ϸ��鵱ǰ�滮���Ρ�һ�����һ����ǰ����ʵ���˷��򴫵�
void planner_reverse_pass() 
{
  uint8_t block_index = block_buffer_head;
  unsigned char tail = block_buffer_tail;
  //Make a local copy of block_buffer_tail, because the interrupt can alter it
	//��һ�����صĸ��ƣ���Ϊ�жϿ��ܻ�ı�����
  CRITICAL_SECTION_START;
 
  CRITICAL_SECTION_END
  
  if(((block_buffer_head-tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1)) > 3) 
  { block_t *block[3] = {   NULL, NULL, NULL         };
    block_index = (block_buffer_head - 3) & (BLOCK_BUFFER_SIZE - 1);
    while(block_index != tail) { 
      block_index = prev_block_index(block_index); 
      block[2]= block[1];
      block[1]= block[0];
      block[0] = &block_buffer[block_index];
      planner_reverse_pass_kernel(block[0], block[1], block[2]);
    }
  }
}

// The kernel called by planner_recalculate() when scanning the plan from first to last entry.
void planner_forward_pass_kernel(block_t *previous, block_t *current, block_t *next) {
  if(!previous) { 
    return; 
  }

  // If the previous block is an acceleration block, but it is not long enough to complete the
  // full speed change within the block, we need to adjust the entry speed accordingly. Entry
  // speeds have already been reset, maximized, and reverse planned by reverse planner.
  // If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
  if (!previous->nominal_length_flag) {
    if (previous->entry_speed < current->entry_speed) {
      double entry_speed = min( current->entry_speed,
      max_allowable_speed(-previous->acceleration,previous->entry_speed,previous->millimeters) );

      // Check for junction speed change
      if (current->entry_speed != entry_speed) {
        current->entry_speed = entry_speed;
        current->recalculate_flag = true;
      }
    }
  }
}

// planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This 
// implements the forward pass.
//planner_recalculate()������Ҫ��ϸ��鵱ǰ�滮���Ρ�һ�����һ����ǰ����ʵ���˷��򴫵�
void planner_forward_pass() {
  uint8_t block_index = block_buffer_tail;
  block_t *block[3] = { 
    NULL, NULL, NULL   };

  while(block_index != block_buffer_head) {
    block[0] = block[1];
    block[1] = block[2];
    block[2] = &block_buffer[block_index];
    planner_forward_pass_kernel(block[0],block[1],block[2]);
    block_index = next_block_index(block_index);
  }
  planner_forward_pass_kernel(block[1], block[2], NULL);
}

// Recalculates the trapezoid speed profiles for all blocks in the plan according to the 
// entry_factor for each junction. Must be called by planner_recalculate() after 
// updating the blocks.
//������滮�и���ÿ���ӵ��entry_factorΪ���е�block���¼��������ٶ�����.�ڸ�����blocks��������planner_recalculate()����
void planner_recalculate_trapezoids() {
  int8_t block_index = block_buffer_tail;
  block_t *current;
  block_t *next = NULL;
  
  while(block_index != block_buffer_head) {
    current = next;
    next = &block_buffer[block_index];
    if (current) {
      // Recalculate if current block entry or exit junction speed has changed.
      if (current->recalculate_flag || next->recalculate_flag) {
        // NOTE: Entry and exit factors always > 0 by all previous logic operations.
        calculate_trapezoid_for_block(current, current->entry_speed/current->nominal_speed,
        next->entry_speed/current->nominal_speed);
        current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
      }
    }
    block_index = next_block_index( block_index );
  }
  // Last/newest block in buffer. Exit speed is set with MINIMUM_PLANNER_SPEED. Always recalculated.
  if(next != NULL) {
    calculate_trapezoid_for_block(next, next->entry_speed/next->nominal_speed,
    MINIMUM_PLANNER_SPEED/next->nominal_speed);
    next->recalculate_flag = false;
  }
}

// Recalculates the motion plan according to the following algorithm:
//������������㷨�����¼������滮
//   1. Go over every block in reverse order and calculate a junction speed reduction (i.e. block_t.entry_factor) 
//      so that:
//     a. The junction jerk is within the set limit
//     b. No speed reduction within one block requires faster deceleration than the one, true constant 
//        acceleration.
//   2. Go over every block in chronological order and dial down junction speed reduction values if 
//     a. The speed increase within one block would require faster accelleration than the one, true 
//        constant acceleration.
//
// When these stages are complete all blocks have an entry_factor that will allow all speed changes to 
// be performed using only the one, true constant acceleration, and where no junction jerk is jerkier than 
// the set limit. Finally it will:
//
//   3. Recalculate trapezoids for all blocks.

void planner_recalculate(void) {   
  planner_reverse_pass();
  planner_forward_pass();
  planner_recalculate_trapezoids();
}

void plan_init(void) {
  block_buffer_head = 0;
  block_buffer_tail = 0;
  memset(position, 0, sizeof(position)); // clear position
  previous_speed[0] = 0.0;
  previous_speed[1] = 0.0;
  previous_speed[2] = 0.0;
  previous_speed[3] = 0.0;
  previous_nominal_speed = 0.0;
}




#ifdef AUTOTEMP
void getHighESpeed(void)
{
  static float oldt=0;
  float high;
  uint8_t block_index ;
  float g;
  float t;
  if(!autotemp_enabled){
    return;
  }
  if(degTargetHotend0()+2<autotemp_min) {  //probably temperature set to zero.
    return; //do nothing
  }
  high=0.0;
  block_index = block_buffer_tail;

  while(block_index != block_buffer_head) 
  {
    if((block_buffer[block_index].steps_x != 0) ||
       (block_buffer[block_index].steps_y != 0) ||
       (block_buffer[block_index].steps_z != 0)) 
	  {  float se;
      	 se=((float)(block_buffer[block_index].steps_e)/(float)(block_buffer[block_index].step_event_count))*block_buffer[block_index].nominal_speed;
     	 //se; mm/sec;
	      if(se>high)
	      {
	        high=se;
	      }
      }
    block_index = (block_index+1) & (BLOCK_BUFFER_SIZE - 1);
  }
  g=autotemp_min+high*autotemp_factor;
  t=g;
  if(t<autotemp_min)
    t=autotemp_min;
  if(t>autotemp_max)
    t=autotemp_max;
  if(oldt>t)
  {
    t=AUTOTEMP_OLDWEIGHT*oldt+(1-AUTOTEMP_OLDWEIGHT)*t;
  }
  oldt=t;
  setTargetHotend0(t);
}
#endif

void check_axes_activity(void)
{
  unsigned char x_active = 0;
  unsigned char y_active = 0;  
  unsigned char z_active = 0;
  unsigned char e_active = 0;
  unsigned char tail_fan_speed = fanSpeed;	
//  #ifdef FAN_KICKSTART_TIME
//  unsigned char tail_fan_speed = fanSpeed;
//  #endif
  #ifdef BARICUDA  //���ǻ��
  unsigned char tail_valve_pressure = ValvePressure;
  unsigned char tail_e_to_p_pressure = EtoPPressure;
  #endif
  block_t *block;

  if(block_buffer_tail != block_buffer_head)
  {
    uint8_t block_index = block_buffer_tail;
	#ifdef FAN_KICKSTART_TIME
    tail_fan_speed = block_buffer[block_index].fan_speed;
	#endif
    #ifdef BARICUDA
    tail_valve_pressure = block_buffer[block_index].valve_pressure;
    tail_e_to_p_pressure = block_buffer[block_index].e_to_p_pressure;
    #endif
    while(block_index != block_buffer_head)
    {
      block = &block_buffer[block_index];
      if(block->steps_x != 0) x_active++;
      if(block->steps_y != 0) y_active++;
      if(block->steps_z != 0) z_active++;
      if(block->steps_e != 0) e_active++;
      block_index = (block_index+1) & (BLOCK_BUFFER_SIZE - 1);
    }
  }
  if((DISABLE_X) && (x_active == 0)) disable_x(); 
  if((DISABLE_Y) && (y_active == 0)) disable_y();
  if((DISABLE_Z) && (z_active == 0)) disable_z();
  if((DISABLE_E) && (e_active == 0))
  {
    disable_e0();
    disable_e1();
//    disable_e2(); 
  }
#if defined(FAN_PIN)
    #ifdef FAN_KICKSTART_TIME  //���ǻ��
      static unsigned long fan_kick_end;
      if (tail_fan_speed) 
	  {
		if (fan_kick_end == 0) 
		{
			// Just starting up fan - run at full power.
			fan_kick_end = millis() + FAN_KICKSTART_TIME;
			tail_fan_speed = 255;
		} 
		else if (fan_kick_end > millis())
		{
			// Fan still spinning up.������Ȼ��ת
			tail_fan_speed = 255;
		}
	  } 
	  else 
	 {
		fan_kick_end = 0;
	 }  
    #endif //FAN_KICKSTART_TIME
	 
	#ifdef FAN_SOFT_PWM //(�)
		fanSpeedSoftPwm = tail_fan_speed;
	#else
		//analogWrite(FAN_PIN,tail_fan_speed); //Ӳ��PWM //��ʱδ֧��
	#endif//!FAN_SOFT_PWM
#endif //FAN_PIN 
		
#ifdef AUTOTEMP
  getHighESpeed();
#endif

#ifdef BARICUDA //���ǻ��
  #if defined(HEATER_1_PIN) && HEATER_1_PIN > -1
      analogWrite(HEATER_1_PIN,tail_valve_pressure);
  #endif

  #if defined(HEATER_2_PIN) && HEATER_2_PIN > -1
      analogWrite(HEATER_2_PIN,tail_e_to_p_pressure);
  #endif
#endif
}


float junction_deviation = 0.1;
// Add a new linear movement to the buffer. steps_x, _y and _z is the absolute position in 
// mm. Microseconds specify how many microseconds the move should take to perform. To aid acceleration
// calculation the caller must also provide the physical length of the line in millimeters.
void plan_buffer_line(const float x, const float y, const float z, const float e, float feed_rate, const uint8_t extruder)
{
  // Calculate the buffer head after we push this byte  //
  int next_buffer_head = next_block_index(block_buffer_head);
  long target[4];
  block_t *block ;
  float delta_mm[4];
  float inverse_millimeters;
  float inverse_second;
  int moves_queued;
  float current_speed[4];
  float speed_factor;
  int i;
  float steps_per_mm; 
  float vmax_junction ; 
  float vmax_junction_factor ;
  double v_allowable ;
   unsigned long segment_time;
	float safe_speed;
  // If the buffer is full: good! That means we are well ahead of the robot. 
  // Rest here until there is room in the buffer.
  while(block_buffer_tail == next_buffer_head) //���block�Ƿ��пռ䣬���û����ȴ���ִ��������������
  {
    manage_heater(); 
    manage_inactivity();
		lcd_update();
//    interface_update(); //��Ļ�������		
  }

  // The target position of the tool in absolute steps
  // Calculate target position in absolute steps
  //this should be done after the wait, because otherwise a M92 code within the gcode disrupts this calculation somehow
  target[X_AXIS] = round(x*axis_steps_per_unit[X_AXIS]);  //�������Ŀ��λ�õ�step��
  target[Y_AXIS] = round(y*axis_steps_per_unit[Y_AXIS]);
  target[Z_AXIS] = round(z*axis_steps_per_unit[Z_AXIS]);     
  target[E_AXIS] = round(e*axis_steps_per_unit[E_AXIS]);
	#ifdef DEBUG_PLANNER
	  printf("\n*****DEBUG_PLANNER*****\n");
	  printf("\n���������µ�Ŀ��λ��\n");
		printf("target[X_AXIS]=%ld\n",target[X_AXIS]);
		printf("target[Y_AXIS]=%ld\n",target[Y_AXIS]);
		printf("target[Z_AXIS]=%ld\n",target[Z_AXIS]);
	  printf("target[E_AXIS]=%ld\n",target[E_AXIS]);
	#endif

	//���м��ϵĶ���ʱ���Ҽ���ͷ���¶ȵ���EXTRUDE_MINTEMPʱ��Ϊ������ͷ��ִ�м��϶���
  #ifdef PREVENT_DANGEROUS_EXTRUDE 
  if(target[E_AXIS]!=position[E_AXIS])
  {
    if(degHotend(active_extruder)<EXTRUDE_MINTEMP && !allow_cold_extrude)
    {
      position[E_AXIS]=target[E_AXIS]; //behave as if the move really took place, but ignore E part
      SERIAL_ECHO_START;
      printf(MSG_ERR_COLD_EXTRUDE_STOP);
    }
    //���м��ϵĶ���ʱ���Ҽ����ĳ��ȳ�������ʱ��Ϊ������ͷ��ִ�м��϶���
    #ifdef PREVENT_LENGTHY_EXTRUDE 
    if(labs(target[E_AXIS]-position[E_AXIS])>axis_steps_per_unit[E_AXIS]*EXTRUDE_MAXLENGTH)
    {
      position[E_AXIS]=target[E_AXIS]; //behave as if the move really took place, but ignore E part
      SERIAL_ECHO_START;
      printf(MSG_ERR_LONG_EXTRUDE_STOP);
    }
    #endif
  }
  #endif

  // Prepare to set up new block
  block = &block_buffer[block_buffer_head]; //�趨�µ�block

  // Mark block as not busy (Not executed by the stepper interrupt)
  block->busy = false;

  // Number of steps for each axis
  block->steps_x = labs(target[X_AXIS]-position[X_AXIS]);
  block->steps_y = labs(target[Y_AXIS]-position[Y_AXIS]);
  block->steps_z = labs(target[Z_AXIS]-position[Z_AXIS]);
  block->steps_e = labs(target[E_AXIS]-position[E_AXIS]);
  block->steps_e *= extrudemultiply;
  block->steps_e /= 100;
  block->step_event_count = max(block->steps_x, max(block->steps_y, max(block->steps_z, block->steps_e)));
	#ifdef DEBUG_PLANNER 
	  printf("\n����Ҫ�˶��Ĳ���\n");
		printf("block->steps_x=%ld\n",block->steps_x);
		printf("block->steps_y=%ld\n",block->steps_y);
		printf("block->steps_z=%ld\n",block->steps_z);
		printf("block->steps_e%ld\n",block->steps_e);
		printf("block->step_event_count:%ld\r\n",block->step_event_count);
   #endif
	 
  // Bail(�ͷ�) if this is a zero-length block
  if (block->step_event_count <= dropsegments) //���step_event_count �Ĳ�������dropsegments���������ָ��
  { 
    return; 
  }

  block->fan_speed = fanSpeed;  //�趨���ȵ��ٶ�
  #ifdef BARICUDA
  block->valve_pressure = ValvePressure;
  block->e_to_p_pressure = EtoPPressure;
  #endif

  // Compute direction bits for this block Ϊ���block���㷽��λ 
  block->direction_bits = 0;  //�趨����ķ���
  if (target[X_AXIS] < position[X_AXIS])
  {
    block->direction_bits |= (1<<X_AXIS); 
  }
  if (target[Y_AXIS] < position[Y_AXIS])
  {
    block->direction_bits |= (1<<Y_AXIS); 
  }
  if (target[Z_AXIS] < position[Z_AXIS])
  {
    block->direction_bits |= (1<<Z_AXIS); 
  }
  if (target[E_AXIS] < position[E_AXIS])
  {
    block->direction_bits |= (1<<E_AXIS); 
  }
  
	#ifdef DEBUG_PLANNER
		printf("\n����ķ���λ\n");
		printf("block->direction_bits=%04X\n",block->direction_bits);
	#endif
	
  block->active_extruder = extruder; //�趨����ͷ�ı��

  //ʹ�ܻ�Ծ��
	//enable active axes
  #ifdef COREXY
  if((block->steps_x != 0) || (block->steps_y != 0))
  {	
    enable_x();
    enable_y();
  }
  #else
  if(block->steps_x != 0) 
  {
		enable_x();
  }
  if(block->steps_y != 0) 
  {
		enable_y();
  }
  #endif
#ifndef Z_LATE_ENABLE  //���ǻ��
  if(block->steps_z != 0) enable_z();
#endif

  // Enable all //ʹ�ܼ���ͷ
  if(block->steps_e != 0)
  {
    enable_e0();
    enable_e1();
   // enable_e2(); 
  }
  //�趨feed_rate
  if (block->steps_e == 0)
  {
    if(feed_rate<mintravelfeedrate) feed_rate=mintravelfeedrate;
  }
  else
  {
    if(feed_rate<minimumfeedrate) feed_rate=minimumfeedrate;
  } 
  //������Ჽ�����Ҫ�ƶ���step��
  delta_mm[X_AXIS] = (target[X_AXIS]-position[X_AXIS])/axis_steps_per_unit[X_AXIS];
  delta_mm[Y_AXIS] = (target[Y_AXIS]-position[Y_AXIS])/axis_steps_per_unit[Y_AXIS];
  delta_mm[Z_AXIS] = (target[Z_AXIS]-position[Z_AXIS])/axis_steps_per_unit[Z_AXIS];
  delta_mm[E_AXIS] = ((target[E_AXIS]-position[E_AXIS])/axis_steps_per_unit[E_AXIS])*extrudemultiply/100.0;
	#ifdef DEBUG_PLANNER
		printf("\n����Ҫ�ƶ��ľ���\n");
		printf("delta_mm[X_AXIS]=%f\n",delta_mm[X_AXIS]);
		printf("delta_mm[Y_AXIS]=%f\n",delta_mm[Y_AXIS]);
		printf("delta_mm[Z_AXIS]=%f\n",delta_mm[Z_AXIS]);
		printf("delta_mm[E_AXIS]=%f\n",delta_mm[E_AXIS]);
	#endif
  if ( block->steps_x <=dropsegments && block->steps_y <=dropsegments && block->steps_z <=dropsegments )
  {
    block->millimeters = fabs(delta_mm[E_AXIS]);
  } 
  else
  {
    block->millimeters = sqrt(square(delta_mm[X_AXIS]) + square(delta_mm[Y_AXIS]) + square(delta_mm[Z_AXIS]));
  }
  inverse_millimeters = 1.0/block->millimeters;  // Inverse millimeters to remove multiple divides

  //�����������ٶȣ�mm/s�� 
	// Calculate speed in mm/second for each axis. No divide by zero due to previous checks.
  inverse_second = feed_rate * inverse_millimeters;	
  //���������г̵�block��
  moves_queued=(block_buffer_head-block_buffer_tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
	
	#ifdef DEBUG_PLANNER
	  printf("\n·���ٶȵȲ���\n");
	  printf("block->millimeters=%f\n",block->millimeters);
	  printf("inverse_millimeters=%f\n",inverse_millimeters);	
	  printf("inverse_second=%f\n",inverse_second);
	  printf("moves_queued=%d\n",moves_queued);
	#endif
	
  // slow down when de buffer starts to empty, rather than wait at the corner for a buffer refill
  #ifdef OLD_SLOWDOWN
  if(moves_queued < (BLOCK_BUFFER_SIZE * 0.5) && moves_queued > 1)
    feed_rate = feed_rate*moves_queued / (BLOCK_BUFFER_SIZE * 0.5); 
  #endif

  #ifdef SLOWDOWN
  //segment time im micro seconds
  segment_time = round(1000000.0/inverse_second);
  if ((moves_queued > 1) && (moves_queued < (BLOCK_BUFFER_SIZE * 0.5)))
  {
    if (segment_time < minsegmenttime)
    { // buffer is draining, add extra time.  The amount of time added increases if the buffer is still emptied more.
      inverse_second=1000000.0/(segment_time+round(2*(float)(minsegmenttime-segment_time)/moves_queued));
      #ifdef XY_FREQUENCY_LIMIT
         segment_time = round(1000000.0/inverse_second);
      #endif
    }
  }
	#ifdef DEBUG_PLANNER
	  printf("\n��⾭����SLOWDOWN����inverse_second��ֵ\n");
	  printf("inverse_second=%f\n",inverse_second);
  #endif	
	#endif
  //  END OF SLOW DOWN SECTION 
  
  //����block��nominal_speed(mm/sec)��nominal_rate(step/sec)
  block->nominal_speed = block->millimeters * inverse_second; // (mm/sec) Always > 0
  block->nominal_rate = ceil(block->step_event_count * inverse_second); // (step/sec) Always > 0
	
  // Calculate and limit speed in mm/sec for each axis
	//�������ĵ�ǰ�ٶȼ��ٶȲ���
  speed_factor = 1.0; //factor <=1 do decrease speed
  for(i=0; i < 4; i++)
  {
    current_speed[i] = delta_mm[i] * inverse_second;
    if(fabs(current_speed[i]) > max_feedrate[i])
      speed_factor = min(speed_factor, max_feedrate[i] / fabs(current_speed[i]));
  }
	#ifdef DEBUG_PLANNER
	  printf("\n��ٶ�\n");
    printf("block->nominal_speed=%fmm/s\n",block->nominal_speed);
	  printf("block->nominal_rate=%ldstep/s\n",block->nominal_rate);
	  printf("speed_factor=%f\n",speed_factor);
	#endif

  // Max segement time in us.
#ifdef XY_FREQUENCY_LIMIT
#define MAX_FREQ_TIME (1000000.0/XY_FREQUENCY_LIMIT)
  // Check and limit the xy direction change frequency
  unsigned char direction_change = block->direction_bits ^ old_direction_bits;
  old_direction_bits = block->direction_bits;
  segment_time = lround((float)segment_time / speed_factor);
  
  if((direction_change & (1<<X_AXIS)) == 0)
  {
    x_segment_time[0] += segment_time;
  }
  else
  {
    x_segment_time[2] = x_segment_time[1];
    x_segment_time[1] = x_segment_time[0];
    x_segment_time[0] = segment_time;
  }
  if((direction_change & (1<<Y_AXIS)) == 0)
  {
    y_segment_time[0] += segment_time;
  }
  else
  {
    y_segment_time[2] = y_segment_time[1];
    y_segment_time[1] = y_segment_time[0];
    y_segment_time[0] = segment_time;
  }
  long max_x_segment_time = max(x_segment_time[0], max(x_segment_time[1], x_segment_time[2]));
  long max_y_segment_time = max(y_segment_time[0], max(y_segment_time[1], y_segment_time[2]));
  long min_xy_segment_time =min(max_x_segment_time, max_y_segment_time);
  if(min_xy_segment_time < MAX_FREQ_TIME)
    speed_factor = min(speed_factor, speed_factor * (float)min_xy_segment_time / (float)MAX_FREQ_TIME);
#endif
  
  // Correct the speed  У���ٶ�
	//speed_factor<1ʱ˵��ָ�����feedrate����ϵͳ�����ƣ����¼���nominal_speed��nominal_rate
  if( speed_factor < 1.0)
  {
    for(i=0; i < 4; i++)
    {
      current_speed[i] *= speed_factor;
    }
    block->nominal_speed *= speed_factor;
    block->nominal_rate *= speed_factor;
		#ifdef DEBUG_PLANNER
	  printf("\nУ�����ٶ�\n");
    printf("block->nominal_speed=%fmm/s\n",block->nominal_speed);
	  printf("block->nominal_rate=%ldstep/s\n",block->nominal_rate);
	  #endif
  }
	
  // Compute and limit the acceleration rate for the trapezoid generator.//Ϊ���η��������㲢�޶����ٶ� 
  //����block�ļ��ٶ�acceleration_st(step/sec^2)
  steps_per_mm = block->step_event_count/block->millimeters; //��λ���루mm����Ӧ�Ĳ�����step��
  if(block->steps_x == 0 && block->steps_y == 0 && block->steps_z == 0)
  {
    block->acceleration_st = ceil(retract_acceleration * steps_per_mm); // convert to: acceleration steps/sec^2
  }
  else
  {
    block->acceleration_st = ceil(acceleration * steps_per_mm); // convert to: acceleration steps/sec^2
    // Limit acceleration per axis //Ϊ������ļ��ٶ��޷�
    if(((float)block->acceleration_st * (float)block->steps_x / (float)block->step_event_count) > axis_steps_per_sqr_second[X_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[X_AXIS];
    if(((float)block->acceleration_st * (float)block->steps_y / (float)block->step_event_count) > axis_steps_per_sqr_second[Y_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[Y_AXIS];
    if(((float)block->acceleration_st * (float)block->steps_e / (float)block->step_event_count) > axis_steps_per_sqr_second[E_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[E_AXIS];
    if(((float)block->acceleration_st * (float)block->steps_z / (float)block->step_event_count ) > axis_steps_per_sqr_second[Z_AXIS])
      block->acceleration_st = axis_steps_per_sqr_second[Z_AXIS];
  }
  block->acceleration = block->acceleration_st / steps_per_mm; //���� block ��acceleration (mm/sqr_sec)
  block->acceleration_rate = (long)((float)block->acceleration_st * 8.388608);//����block �� acceleration_rate ��Ϊ���������� step ���ɵļ��ٶ�ֵ
  #ifdef DEBUG_PLANNER
	  printf("block->acceleration_st=%ld\n",block->acceleration_st);
	  printf("block->acceleration=%f\n",block->acceleration);
  	printf("block->acceleration_rate=%ld\n",block->acceleration_rate);
	#endif
	
	
#if 0  // Use old jerk for now
  // Compute path unit vector
  double unit_vec[3];

  unit_vec[X_AXIS] = delta_mm[X_AXIS]*inverse_millimeters;
  unit_vec[Y_AXIS] = delta_mm[Y_AXIS]*inverse_millimeters;
  unit_vec[Z_AXIS] = delta_mm[Z_AXIS]*inverse_millimeters;

  // Compute maximum allowable entry speed at junction by centripetal acceleration approximation.
  // Let a circle be tangent to both previous and current path line segments, where the junction
  // deviation is defined as the distance from the junction to the closest edge of the circle,
  // colinear with the circle center. The circular segment joining the two paths represents the
  // path of centripetal acceleration. Solve for max velocity based on max acceleration about the
  // radius of the circle, defined indirectly by junction deviation. This may be also viewed as
  // path width or max_jerk in the previous grbl version. This approach does not actually deviate
  // from path, but used as a robust way to compute cornering speeds, as it takes into account the
  // nonlinearities of both the junction angle and junction velocity.
  double vmax_junction = MINIMUM_PLANNER_SPEED; // Set default max junction speed

  // Skip first block or when previous_nominal_speed is used as a flag for homing and offset cycles.
  if ((block_buffer_head != block_buffer_tail) && (previous_nominal_speed > 0.0)) {
    // Compute cosine of angle between previous and current path. (prev_unit_vec is negative)
    // NOTE: Max junction velocity is computed without sin() or acos() by trig half angle identity.
    double cos_theta = - previous_unit_vec[X_AXIS] * unit_vec[X_AXIS]
      - previous_unit_vec[Y_AXIS] * unit_vec[Y_AXIS]
      - previous_unit_vec[Z_AXIS] * unit_vec[Z_AXIS] ;

    // Skip and use default max junction speed for 0 degree acute junction.
    if (cos_theta < 0.95) {
      vmax_junction = min(previous_nominal_speed,block->nominal_speed);
      // Skip and avoid divide by zero for straight junctions at 180 degrees. Limit to min() of nominal speeds.
      if (cos_theta > -0.95) {
        // Compute maximum junction velocity based on maximum acceleration and junction deviation
        double sin_theta_d2 = sqrt(0.5*(1.0-cos_theta)); // Trig half angle identity. Always positive.
        vmax_junction = min(vmax_junction,
        sqrt(block->acceleration * junction_deviation * sin_theta_d2/(1.0-sin_theta_d2)) );
      }
    }
  }
#endif
	
  // Start with a safe speed //�԰�ȫ�ٶ���Ϊ��ʼ
  vmax_junction = max_xy_jerk/2; 
  vmax_junction_factor = 1.0; 
  if(fabs(current_speed[Z_AXIS]) > max_z_jerk/2) 
    vmax_junction = min(vmax_junction, max_z_jerk/2);
  if(fabs(current_speed[E_AXIS]) > max_e_jerk/2) 
    vmax_junction = min(vmax_junction, max_e_jerk/2);
  vmax_junction = min(vmax_junction, block->nominal_speed);
  safe_speed = vmax_junction;

  if ((moves_queued > 1) && (previous_nominal_speed > 0.0001)) {
    float jerk = sqrt(pow((current_speed[X_AXIS]-previous_speed[X_AXIS]), 2)+pow((current_speed[Y_AXIS]-previous_speed[Y_AXIS]), 2));
    //    if((fabs(previous_speed[X_AXIS]) > 0.0001) || (fabs(previous_speed[Y_AXIS]) > 0.0001)) {
    vmax_junction = block->nominal_speed;
    //    }
    if (jerk > max_xy_jerk) {
      vmax_junction_factor = (max_xy_jerk/jerk);
    } 
    if(fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS]) > max_z_jerk) {
      vmax_junction_factor= min(vmax_junction_factor, (max_z_jerk/fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS])));
    } 
    if(fabs(current_speed[E_AXIS] - previous_speed[E_AXIS]) > max_e_jerk) {
      vmax_junction_factor = min(vmax_junction_factor, (max_e_jerk/fabs(current_speed[E_AXIS] - previous_speed[E_AXIS])));
    } 
    vmax_junction = min(previous_nominal_speed, vmax_junction * vmax_junction_factor); // Limit speed to max previous speed
  }
  block->max_entry_speed = vmax_junction;

  //��ʼ��block�ĳ�ʼ�ٶȣ����ݼ��ٽ׶��û������MINIMUM_PLANNER_SPEED������
	// Initialize block entry speed. Compute based on deceleration to user-defined MINIMUM_PLANNER_SPEED.
  //�ڼ��ٶȺ��ƶ�����ȷ��ʱ��entry_speed�����������ٶ�����ǣ����ȼ��ٵ��˶���ĩ�ٶ�MINIMUM_PLANNER_SPEED����V0^2 = V1^2 - (-a)*d��
	v_allowable = max_allowable_speed(-block->acceleration,MINIMUM_PLANNER_SPEED,block->millimeters); //�ܹ��ﵽĿ���ٶȣ����������ʼ�ٶȡ�
  block->entry_speed = min(vmax_junction, v_allowable);

  // Initialize planner efficiency flags
  // Set flag if block will always reach maximum junction speed regardless of entry/exit speeds.
  // If a block can de/ac-celerate from nominal speed to zero within the length of the block, then
  // the current block and next block junction speeds are guaranteed to always be at their maximum
  // junction speeds in deceleration and acceleration, respectively. This is due to how the current
  // block nominal speed limits both the current and next maximum junction speeds. Hence, in both
  // the reverse and forward planners, the corresponding block junction speed will always be at the
  // the maximum junction speed and may always be ignored for any speed reduction checks.
  //���nominal_speedС��v_allowable����·���滮ʱ��block�������ٶȻ����¼���
	if (block->nominal_speed <= v_allowable) { 
    block->nominal_length_flag = true; 
  }
  else { 
    block->nominal_length_flag = false; 
  }
  block->recalculate_flag = true; // Always calculate trapezoid for new block //����Ϊ�µ�block��������

  // Update previous path unit_vector and nominal speed //�����ϴ�·���ĵ�λ�����Ͷ�ٶ�
  memcpy(previous_speed, current_speed, sizeof(previous_speed)); // previous_speed[] = current_speed[]
  previous_nominal_speed = block->nominal_speed;
  
	#ifdef DEBUG_PLANNER
		printf("safe_speed=%f\n",safe_speed);
		printf("block->max_entry_speed=%f\n",block->max_entry_speed);
		printf("v_allowable=%f\n",v_allowable);
		printf("block->entry_speed=%f\n",block->entry_speed);
		printf("block->nominal_length_flag=%d\n",block->nominal_length_flag);
	#endif

#ifdef ADVANCE
  // Calculate advance rate
  if((block->steps_e == 0) || (block->steps_x == 0 && block->steps_y == 0 && block->steps_z == 0)) {
    block->advance_rate = 0;
    block->advance = 0;
  }
  else {
    long acc_dist = estimate_acceleration_distance(0, block->nominal_rate, block->acceleration_st);
    float advance = (STEPS_PER_CUBIC_MM_E * EXTRUDER_ADVANCE_K) * 
      (current_speed[E_AXIS] * current_speed[E_AXIS] * EXTRUTION_AREA * EXTRUTION_AREA)*256;
    block->advance = advance;
    if(acc_dist == 0) {
      block->advance_rate = 0;
    } 
    else {
      block->advance_rate = advance / (float)acc_dist;
    }
  }
  /*
    SERIAL_ECHO_START;
   SERIAL_ECHOPGM("advance :");
   SERIAL_ECHO(block->advance/256.0);
   SERIAL_ECHOPGM("advance rate :");
   SERIAL_ECHOLN(block->advance_rate/256.0);
   */
#endif // ADVANCE
  //�������μӼ���
  calculate_trapezoid_for_block(block, block->entry_speed/block->nominal_speed,
  safe_speed/block->nominal_speed);
  #ifdef DEBUG_PLANNER
	  printf("\nΪblock��������β���\n");
		printf("block->accelerate_until->initial_rate=%ld\n",block->accelerate_until);
		printf("block->decelerate_after=%ld\n",block->decelerate_after);
		printf("block->initial_rate=%ld\n",block->initial_rate);
	  printf("block->final_rate=%ld\n",block->final_rate);
	#endif
	  
  // Move buffer head
  block_buffer_head = next_buffer_head;
  // Update position ��¼Ŀǰ��λ��
  memcpy(position, target, sizeof(target)); // position[] = target[]
  //���¶����е�block���켣�滮
  planner_recalculate();
	#ifdef DEBUG_PLANNER
	  printf("\n·�����¹滮������β���\n");
		printf("block->accelerate_until->initial_rate=%ld\n",block->accelerate_until);
		printf("block->decelerate_after=%ld\n",block->decelerate_after);
		printf("block->initial_rate=%ld\n",block->initial_rate);
	  printf("block->final_rate=%ld\n",block->final_rate);
	#endif
  //ʹ�ܲ�������ж�
  st_wake_up();
}

void plan_set_position(const float x, const float y, const float z, const float e) //�趨�����λ��
{
  position[X_AXIS] = round(x*axis_steps_per_unit[X_AXIS]);
  position[Y_AXIS] = round(y*axis_steps_per_unit[Y_AXIS]);
  position[Z_AXIS] = round(z*axis_steps_per_unit[Z_AXIS]);     
  position[E_AXIS] = round(e*axis_steps_per_unit[E_AXIS]);  
  st_set_position(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[E_AXIS]);
  previous_nominal_speed = 0.0; // Resets planner junction speeds. Assumes start from rest.
  previous_speed[0] = 0.0;
  previous_speed[1] = 0.0;
  previous_speed[2] = 0.0;
  previous_speed[3] = 0.0;
}

void plan_set_e_position(const float e) //�趨��ͷλ��
{
  position[E_AXIS] = round(e*axis_steps_per_unit[E_AXIS]);  
  st_set_e_position(position[E_AXIS]);
}

uint8_t movesplanned()
{
  return (block_buffer_head-block_buffer_tail + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
}

void allow_cold_extrudes(bool allow) //��������ͷ
{
#ifdef PREVENT_DANGEROUS_EXTRUDE
  allow_cold_extrude=allow;
#endif
}

// Calculate the steps/s^2 acceleration rates, based on the mm/s^s //�����ٶȵĵ�λmm/s^2ת��Ϊstep/s^2
void reset_acceleration_rates(void) //���ٶȵ�λת��
{	int8_t i;
	for(i=0; i < NUM_AXIS; i++)
        {
        axis_steps_per_sqr_second[i] = max_acceleration_units_per_sq_second[i] * axis_steps_per_unit[i];
        }
}
void plan_discard_current_block(void)  //������ǰ��block
{  
  if (block_buffer_head != block_buffer_tail) {
    block_buffer_tail = (block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1);  
  }
}

// Gets the current block. Returns NULL if buffer empty //��ȡ��ǰblock�����bufferΪ�շ���NULL
block_t *plan_get_current_block(void) //��ȡ��ǰblock
{
  if (block_buffer_head == block_buffer_tail) { 
    return(NULL); 
  }
  else
  {
  block_t *block = &block_buffer[block_buffer_tail];
  block->busy = true;
  return(block);
  }
}
// Gets the current block. Returns NULL if buffer empty //��ȡ��ǰblock�����bufferΪ�շ���NULL
bool blocks_queued(void) //��ȡ��ǰblock,
{
  if (block_buffer_head == block_buffer_tail) { 
    return false; 
  }
  else
    return true;
}
