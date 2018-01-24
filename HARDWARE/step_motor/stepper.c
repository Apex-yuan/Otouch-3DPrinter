#include "stepper.h"
#include "marlin_main.h"
#include "language.h"
//#include "Configuration_adv.h"
//#include "Configuration.h"
//#include "speed_lookuptable.h" //ʵ����û�в��ò��ķ���
#include "temperature.h"
#include "usart.h"
#include "timer.h"
//#include "spi.h"

#ifdef DEBUG_STEPPER
  unsigned int add_interrupt_time = 0; //Ϊ����ʱͳ�������������ٶ�ʱ��
#endif

//�����������꣬����������ֵ�λ�������ţ��ڴ�û���õ����ֵ�λ����
//���������Ŷ��巽ʽ��������STM32���������Ҫ�����ֵ�λ�����ھ��忼����ôӦ�á�
//#define DIGIPOT_CHANNELS {4,1,0,2,3} // X Y Z E0 E1 digipot channels to stepper driver mapping

//�洢��ȡ���Ĳ������������ϸ��ֵ��û���õ��������ϸ�֣�ֱ��Ӳ������Ϊ16ϸ�֣����������ȫ��ʼ��Ϊ16
static uint8_t subsection_x_value=16;
static uint8_t subsection_y_value=16;
static uint8_t subsection_z_value=16;
static uint8_t subsection_e0_value=16;
static uint8_t subsection_e1_value=16;


//===========================================================================
//=============================public variables  ============================
//===========================================================================
block_t *current_block;  // A pointer to the block currently being traced


//===========================================================================
//=============================private variables ============================
//===========================================================================
//static makes it inpossible to be called from outside of this file by extern.!

// Variables used by The Stepper Driver Interrupt
static unsigned char out_bits;        // The next stepping-bits to be output
static long counter_x,       // Counter variables for the bresenham line tracer
            counter_y, 
            counter_z,       
            counter_e;
volatile static unsigned long step_events_completed; //block����ɶ�// The number of step events executed in the current block //�ڵ�ǰblock�б�ִ�еĲ��¼�������
#ifdef ADVANCE  //���ǻ��
  static long advance_rate, advance, final_advance = 0;
  static long old_advance = 0;
  static long e_steps[3];
#endif
static long acceleration_time, deceleration_time;
//static unsigned long accelerate_until, decelerate_after, acceleration_rate, initial_rate, final_rate, nominal_rate;
static unsigned short acc_step_rate; // needed for deccelaration start point
static char step_loops;
static unsigned short TIMER4_nominal;
static unsigned short step_loops_nominal;

volatile long endstops_trigsteps[3]={0,0,0};
volatile long endstops_stepsTotal,endstops_stepsDone;
static volatile bool endstop_x_hit=false;
static volatile bool endstop_y_hit=false;
static volatile bool endstop_z_hit=false;

#ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED  //���ǻ��
bool abort_on_endstop_hit = false;
#endif //ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED

static bool old_x_min_endstop=false;
static bool old_x_max_endstop=false;
static bool old_y_min_endstop=false;
static bool old_y_max_endstop=false;
static bool old_z_min_endstop=false;
static bool old_z_max_endstop=false;

static bool check_endstops = true;

volatile long count_position[NUM_AXIS] = { 0, 0, 0, 0}; //���ڼ�¼�������˶���ʼ�������ʼ�˶����λ��
volatile signed char count_direction[NUM_AXIS] = { 1, 1, 1, 1}; //��¼����ķ������巽��ֵ�����岽�������������Ǹ����λ�ã�

//===========================================================================
//=============================functions         ============================
//===========================================================================

#define CHECK_ENDSTOPS  if(check_endstops) //������λ���ؼ��
//ԭ��arduino������������������Ķ��������û�������ɵģ����������������ٶȣ�������Ķ������C���Եķ�ʽ������Ч�ʿ����Եͣ���ʵ�ֵĹ���һ�¡�
#define MultiU24X24toH16(intRes, longIn1, longIn2) intRes= ((uint64_t)(longIn1) * (longIn2)) >> 24 //intRes = longIn1 * longIn2 >> 24
#define MultiU16X8toH16(intRes, charIn1, intIn2) intRes = ((charIn1) * (intIn2)) >> 16  // intRes = intIn1 * intIn2 >> 16

void checkHitEndstops(void)
{
 if( endstop_x_hit || endstop_y_hit || endstop_z_hit) {
   SERIAL_ECHO_START;
   printf(MSG_ENDSTOPS_HIT);
   if(endstop_x_hit) {
     printf(" X:%f",(float)endstops_trigsteps[X_AXIS]/axis_steps_per_unit[X_AXIS]);  //endstops_trigsteps[X_AXIS]�洢����Ϣ��ʲô��  
   }
   if(endstop_y_hit) {
     printf(" Y:%f",(float)endstops_trigsteps[Y_AXIS]/axis_steps_per_unit[Y_AXIS]);  
   }
   if(endstop_z_hit) {
     printf(" Z:%f",(float)endstops_trigsteps[Z_AXIS]/axis_steps_per_unit[Z_AXIS]); 
   }
   printf("\n");
   endstop_x_hit=false;
   endstop_y_hit=false;
   endstop_z_hit=false;
#ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED //���ǻ��
   if (abort_on_endstop_hit)
   {
     card.sdprinting = false;
     card.closefile();
     quickStop();
     setTargetHotend0(0);
     setTargetHotend1(0);
     setTargetHotend2(0);
   }
#endif
 }
}

void endstops_hit_on_purpose(void)  //��λ������Ŀ��λ�ô����󣬽��临λ
{
  endstop_x_hit=false;
  endstop_y_hit=false;
  endstop_z_hit=false;
}

void enable_endstops(bool check)  //�Ƿ�ʹ����λ���ؼ�⣬����״̬��Ϣ���浽check_endstops������
{
  check_endstops = check;
}

//         __________________________
//        /|                        |\     _________________         ^
//       / |                        | \   /|               |\        |
//      /  |                        |  \ / |               | \       s
//     /   |                        |   |  |               |  \      p
//    /    |                        |   |  |               |   \     e
//   +-----+------------------------+---+--+---------------+----+    e
//   |               BLOCK 1            |      BLOCK 2          |    d
//
//                           time ----->
// 
//  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates 
//  first block->accelerate_until step_events_completed, then keeps going at constant speed until 
//  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
//  The slope of acceleration is calculated with the leib ramp alghorithm.

void st_wake_up(void)  //���Ѳ������������TIMx�ж�
{ 
  ENABLE_STEPPER_DRIVER_INTERRUPT();  
}

void step_wait(void)  //�����ȴ�
{
   u8 i;
    for( i=0; i < 6; i++){
    }
}
  

unsigned short calc_timer(unsigned short step_rate)  //�����ж�ʱ��
{
  unsigned short timer;
  if(step_rate > MAX_STEP_FREQUENCY) step_rate = MAX_STEP_FREQUENCY;//�������Ƶ�����ֵ�޷���40KHZ��
  
  if(step_rate > 20000) { // If steprate > 20kHz >> step 4 times
    step_rate = (step_rate >> 2)&0x3fff;  //�ü��㹫ʽ��ʲô��˼��
    step_loops = 4;  //ȫ�ֱ�������¼ѭ������
  }
  else if(step_rate > 10000) { // If steprate > 10kHz >> step 2 times
    step_rate = (step_rate >> 1)&0x7fff; //�ü��㹫ʽ��ʲô��˼��
    step_loops = 2;
  }
  else {
    step_loops = 1;
  } 
  
  if(step_rate < 32) step_rate = 32;
  timer = 2000000/step_rate - 1;
	 
  if(timer < 100) { timer = 100; printf(MSG_STEPPER_TO_HIGH); printf("%d",step_rate); }//(20kHz this should never happen)
  return timer;
}

// Initializes the trapezoid generator from the current block. Called whenever a new 
// block begins. //��ʼ����ǰblock�����η�������ÿ��һ���µ�block��ʼʱ��Ҫ����
void trapezoid_generator_reset(void)
{
  #ifdef ADVANCE  //���ǻ��
    advance = current_block->initial_advance;
    final_advance = current_block->final_advance;
    // Do E steps + advance steps
    e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
    old_advance = advance >>8;  
  #endif
	
  deceleration_time = 0; //�趨���ٶε�ʱ��
  // step_rate to timer interval
  TIMER4_nominal = calc_timer(current_block->nominal_rate); //�����ٶȶ��жϵ�ʱ��
  // make a note of the number of step loops required at nominal speed //�����ڶ�ٶ�������Ĳ���ѭ������
  step_loops_nominal = step_loops;
  acc_step_rate = current_block->initial_rate; //�趨���ٶȶε��ٶ�
  acceleration_time = calc_timer(acc_step_rate); //������ٶȶ��жϵ�ʱ��
   TIM_SetAutoreload(TIM4, acceleration_time);
	#ifdef DEBUG_STEPPER
	printf("\n***trapezoid_generator_reset***\n");
	//Ϊ��������ͳ�Ʒ���ȷ����ӡ��ʽ���£��磺��ʼ�ٶ�=xxxx  ʱ���=
	printf("��ʼ�ٶ�=%6d  ",acc_step_rate);
	printf("�ж�ʱ��=%5ld  ",acceleration_time);
	add_interrupt_time += acceleration_time;
  printf("ʱ��=%8d\n",add_interrupt_time);
	#endif
	
//    SERIAL_ECHO_START;
//    SERIAL_ECHOPGM("advance :");
//    SERIAL_ECHO(current_block->advance/256.0);
//    SERIAL_ECHOPGM("advance rate :");
//    SERIAL_ECHO(current_block->advance_rate/256.0);
//    SERIAL_ECHOPGM("initial advance :");
//  SERIAL_ECHO(current_block->initial_advance/256.0);
//    SERIAL_ECHOPGM("final advance :");
//    SERIAL_ECHOLN(current_block->final_advance/256.0);
    
}


// "The Stepper Driver Interrupt" - This timer interrupt is the workhorse���ظ��ɣ�.  
// It pops blocks from the block_buffer and executes them by pulsing the stepper pins appropriately.
//��block��������ȡ��block���ʵ���ͨ�����岽����ִ������
//��ʱ��4�жϷ������
void TIM4_IRQHandler(void)   //TIM4�ж�
{ 
  if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
  {	  
	  uint8_t i; //�����õ��ľֲ��������壨keil���Ҫ��ֲ������ڶ���ʱ���������ǰ��Ĵ����ţ�����ֱ�Ӷ��嵽ʹ��֮ǰ������ᱨ��	  
	  unsigned short timer;
      unsigned short step_rate;
	  
	  TIM_ClearITPendingBit(TIM4, TIM_IT_Update );  //���TIMx���жϴ�����λ:TIM �ж�Դ  
  
	  // If there is no current block, attempt to pop one from the buffer//���û�е�ǰblock�����Դӻ�������ȡ��	 
	  if (current_block == NULL) // Anything in the buffer?
	  {   
		current_block = plan_get_current_block(); //�ӻ������ж�ȡblock
		  
		if (current_block != NULL) 
		{
		  current_block->busy = true;  //���block״̬Ϊ��æ
		  trapezoid_generator_reset();  //���η�������λ
		  counter_x = -(current_block->step_event_count >> 1);  //step_event_count������Ҫ�ƶ����������ֵ //��֪��Ϊʲô��ô��ֵ
		  counter_y = counter_x;
		  counter_z = counter_x;
		  counter_e = counter_x;
		  step_events_completed = 0; //���㣬���ڼ�¼��ǰblock����ɶ�
		  
		  #ifdef Z_LATE_ENABLE //���ǻ��
			if(current_block->steps_z > 0) {
			  enable_z();
			  TIM_SetAutoreload(TIM4, 2000-1);//1ms wait  
			  return;
			}
		  #endif  //Z_LATE_ENABLE    

		} 
		else 
		{
		  TIM_SetAutoreload(TIM4, 2000-1); //�趨��ʱ������װ�صļ�������ֵ��2000�ڴ�Ϊ1ms
		}    
	  } 

  if (current_block != NULL) 
  {
    // Set directions,This should be done once during init of trapezoid. Endstops -> interrupt
    out_bits = current_block->direction_bits; //��ȡblock�еĸ���ķ��򡣸���ķ���ֵ�Ǵ洢��һ�����е�

    // Set direction en check limit switches //�趨����ʹ����λ���ؼ��
    if ((out_bits & (1<<X_AXIS)) != 0) //ȡ��X��ķ���0�������� 1��������// stepping along -X axis  //��-X���򲽽�
	{   
      #if !defined COREXY  
        X_DIR_PIN = INVERT_X_DIR;
      #endif //NOT COREXY
      count_direction[X_AXIS] = -1;
      if(check_endstops) //CHECK_ENDSTOPS������λ���ؼ��
      {
		#if defined X_MIN_PIN  //��������Сֵ������λ����
          bool x_min_endstop= X_MIN_PIN != X_MIN_ENDSTOP_INVERTING ; //δ�������г̿���ʱֵΪ0��������ֵΪ1
          if(x_min_endstop && old_x_min_endstop && (current_block->steps_x > 0)) 
		      {
            endstops_trigsteps[X_AXIS] = count_position[X_AXIS];  //��¼��ǰλ��
            endstop_x_hit=true;  //�趨x��endstop��״̬
            step_events_completed = current_block->step_event_count; //�趨��block�Ѿ����
          }
          old_x_min_endstop = x_min_endstop;
		#endif //defined X_MIN_PIN
      }
    }
    else  // X��������
	{ 
      #if !defined COREXY  
       X_DIR_PIN=!INVERT_X_DIR;
      #endif //NOT COREXY   
      count_direction[X_AXIS]=1;
      if(check_endstops) //CHECK_ENDSTOPS������λ���ؼ�� 
      {  
		#if defined X_MAX_PIN
          bool x_max_endstop= X_MAX_PIN != X_MAX_ENDSTOP_INVERTING;
          if(x_max_endstop && old_x_max_endstop && (current_block->steps_x > 0))
		      {
            endstops_trigsteps[X_AXIS] = count_position[X_AXIS];
            endstop_x_hit=true;
            step_events_completed = current_block->step_event_count;
          }
          old_x_max_endstop = x_max_endstop;
		#endif
      }
    }

    if ((out_bits & (1<<Y_AXIS)) != 0)  // Y�Ḻ����
	{   
      #if !defined COREXY  //NOT COREXY
        Y_DIR_PIN=INVERT_Y_DIR;
      #endif
      count_direction[Y_AXIS] = -1;
      if(check_endstops) //CHECK_ENDSTOPS������λ���ؼ��
      {
        #if defined(Y_MIN_PIN)
          bool y_min_endstop=Y_MIN_PIN != Y_MIN_ENDSTOP_INVERTING;
          if(y_min_endstop && old_y_min_endstop && (current_block->steps_y > 0)) 
		      {
            endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
            endstop_y_hit=true;
            step_events_completed = current_block->step_event_count;
          }
          old_y_min_endstop = y_min_endstop;
        #endif
      }
    }
    else  //Y�������� 
	{ 
      #if !defined COREXY  //NOT COREXY
        Y_DIR_PIN=!INVERT_Y_DIR;
      #endif
      count_direction[Y_AXIS]=1;
      if(check_endstops) //CHECK_ENDSTOPS������λ���ؼ��
      {
        #if defined(Y_MAX_PIN)// && Y_MAX_PIN > -1
          bool y_max_endstop=Y_MAX_PIN != Y_MAX_ENDSTOP_INVERTING;
          if(y_max_endstop && old_y_max_endstop && (current_block->steps_y > 0))
		      {
            endstops_trigsteps[Y_AXIS] = count_position[Y_AXIS];
            endstop_y_hit=true;
            step_events_completed = current_block->step_event_count;
          }
          old_y_max_endstop = y_max_endstop;
        #endif
      }
    } 
    
    if ((out_bits & (1<<Z_AXIS)) != 0) //Z�Ḻ����
	{ 
      Z_DIR_PIN=INVERT_Z_DIR;     
      count_direction[Z_AXIS] = -1;
      if(check_endstops) //CHECK_ENDSTOPS������λ���ؼ��
      {
        #if defined(Z_MIN_PIN)
          bool z_min_endstop= Z_MIN_PIN != Z_MIN_ENDSTOP_INVERTING;
          if(z_min_endstop && old_z_min_endstop && (current_block->steps_z > 0)) 
		      {
            endstops_trigsteps[Z_AXIS] = count_position[Z_AXIS];
            endstop_z_hit=true;
            step_events_completed = current_block->step_event_count;
          }
          old_z_min_endstop = z_min_endstop;
        #endif
      }
    }
    else //Z�������� 
	{ 
      Z_DIR_PIN=!INVERT_Z_DIR;
      count_direction[Z_AXIS] = 1;
      if(check_endstops) //CHECK_ENDSTOPS������λ���ؼ��
      {
        #if defined(Z_MAX_PIN)
          bool z_max_endstop=Z_MAX_PIN != Z_MAX_ENDSTOP_INVERTING;
          if(z_max_endstop && old_z_max_endstop && (current_block->steps_z > 0)) 
		      {
            endstops_trigsteps[Z_AXIS] = count_position[Z_AXIS];
            endstop_z_hit=true;
            step_events_completed = current_block->step_event_count;
          }
          old_z_max_endstop = z_max_endstop;
        #endif
      }
    }
	
    #ifndef ADVANCE	  //���ǻ��
	
      if ((out_bits & (1<<E_AXIS)) != 0) {  // -direction
        REV_E_DIR();
        count_direction[E_AXIS]=-1;
      }
      else { // +direction
        NORM_E_DIR();
        count_direction[E_AXIS]=1;
      } 
    #endif //!ADVANCE   
	  
//���������һ���ж����ڲ������תһ�������ж����ڵĶ�ʱƵ�ʼ�Ϊ�������������Ƶ�ʡ�
//block�е�step_event_count�����¼�������������Ҫ�ƶ����������ֵ��������ɸ�block	  
	// Take multiple steps per interrupt (For high speed moves)	//ÿ���ж������߶ಽ	
    for(i=0; i < step_loops; i++)  //step_loops������clac_timer()������ȷ��
	{  

      #ifdef ADVANCE  //���ǻ��	  
      counter_e += current_block->steps_e;
      if (counter_e > 0) {
        counter_e -= current_block->step_event_count;
        if ((out_bits & (1<<E_AXIS)) != 0) { // - direction
          e_steps[current_block->active_extruder]--;
        }
        else {
          e_steps[current_block->active_extruder]++;
        }
      }   
	  
      #endif //ADVANCE

     
	  #if !defined COREXY   
        counter_x += current_block->steps_x;
        if (counter_x > 0)   //steps����step_event_count��һ��
		    {
          X_STEP_PIN= !INVERT_X_STEP_PIN; //true
          counter_x -= current_block->step_event_count;  //��䲻֪����ʲô��˼��
          count_position[X_AXIS]+=count_direction[X_AXIS];  //����X��ĵ�ǰλ�� 
          X_STEP_PIN= INVERT_X_STEP_PIN; //false
        }
  
        counter_y += current_block->steps_y;
        if (counter_y > 0) 
		    {
          Y_STEP_PIN= !INVERT_Y_STEP_PIN; //true
          counter_y -= current_block->step_event_count; 
          count_position[Y_AXIS]+=count_direction[Y_AXIS]; //����Y��ĵ�ǰλ�� 
          Y_STEP_PIN= INVERT_Y_STEP_PIN; //false
        }
      #endif  //!defined COREXY 
  
      counter_z += current_block->steps_z;
      if (counter_z > 0) 
	    {
        Z_STEP_PIN= !INVERT_Z_STEP_PIN; //true      
        counter_z -= current_block->step_event_count;
        count_position[Z_AXIS]+=count_direction[Z_AXIS]; //����Z��ĵ�ǰλ�� 
        Z_STEP_PIN= INVERT_Z_STEP_PIN; //false
      }

      #ifndef ADVANCE  	  
        counter_e += current_block->steps_e;
        if (counter_e > 0) 
		    {
          WRITE_E_STEP(!INVERT_E_STEP_PIN); //true
          counter_e -= current_block->step_event_count;
          count_position[E_AXIS]+=count_direction[E_AXIS]; //����E��ĵ�ǰλ�� 
          WRITE_E_STEP(INVERT_E_STEP_PIN); //false
        }		
      #endif //!ADVANCE	 
      step_events_completed += 1;   //����block�Ľ���
      if(step_events_completed >= current_block->step_event_count) break;
    }
	   
	// Calculare new timer value //�����µĶ�ʱ�жϵ�ʱ��  
    if (step_events_completed <= (unsigned long int)current_block->accelerate_until) //���μ��ٽ׶�
	{ 
       MultiU24X24toH16(acc_step_rate, acceleration_time, current_block->acceleration_rate);
       acc_step_rate += current_block->initial_rate;
      
      // upper limit //���ٶȵ�����ֵ
      if(acc_step_rate > current_block->nominal_rate)
         acc_step_rate = current_block->nominal_rate;

      // step_rate to timer interval //�������Ƶ�ʶ�Ӧ��ʱ����
      timer = calc_timer(acc_step_rate); //�����´��жϵ�ʱ��
      TIM_SetAutoreload(TIM4, timer);
      acceleration_time += timer;
			#ifdef DEBUG_STEPPER
				//printf("****���ٽ׶�***\n");
			  //Ϊ��������ͳ�Ʒ���ȷ����ӡ��ʽ���£��磺��ʼ�ٶ�=xxxx  ʱ���=
				printf("�����ٶ�=%6d  ",acc_step_rate); //ÿ���жϵĲ�������ٶȣ���step/s��
				printf("�ж�ʱ��=%5d  ",timer);
			  add_interrupt_time += timer;
				printf("ʱ��=%8d\n",add_interrupt_time);
			#endif
			
      #ifdef ADVANCE  //���ǻ�� 
        for(i=0; i < step_loops; i++) 
	    {
          advance += advance_rate;
        }
        //if(advance > current_block->advance) advance = current_block->advance;
        // Do E steps + advance steps
        e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
        old_advance = advance >>8;        
      #endif
		
    } 
    else if (step_events_completed > (unsigned long int)current_block->decelerate_after) //���μ��ٽ׶�
	{
	      MultiU24X24toH16(step_rate, deceleration_time, current_block->acceleration_rate);
	      
	      if(step_rate > acc_step_rate) // Check step_rate stays positive
		  { 
	        step_rate = current_block->final_rate;
	      }
	      else 
		  {
	        step_rate = acc_step_rate - step_rate; //����Ŀǰ���ٶ�// Decelerate from aceleration end point.//�Ӽ��ٶȽ����㿪ʼ����
	      }
	
	      // lower limit
	      if(step_rate < current_block->final_rate) 
			  step_rate = current_block->final_rate;
	        	
	      // step_rate to timer interval
	      timer = calc_timer(step_rate);
	      TIM_SetAutoreload(TIM4, timer); //�����´��жϵ�ʱ��
	      deceleration_time += timer;
				#ifdef DEBUG_STEPPER
					//printf("****���ٽ׶�***\n");
					//Ϊ��������ͳ�Ʒ���ȷ����ӡ��ʽ���£��磺��ʼ�ٶ�=xxxx  ʱ���=
					printf("�����ٶ�=%6d  ",step_rate); //ÿ���жϵĲ�������ٶȣ���step/s��
					printf("�ж�ʱ��=%5d  ",timer);
				  add_interrupt_time += timer;
					printf("ʱ��=%8d\n",add_interrupt_time);
			  #endif

	      #ifdef ADVANCE   
	        for(i=0; i < step_loops; i++) {
	          advance -= advance_rate;
	        }
	        if(advance < final_advance) advance = final_advance;
	        // Do E steps + advance steps
	        e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
	        old_advance = advance >>8;   			
	      #endif //ADVANCE	
	}
	else 
	{
	   TIM_SetAutoreload(TIM4,TIMER4_nominal-1);
	   // ensure we're running at the correct step rate, even if we just came off an acceleration
     step_loops = step_loops_nominal;
		#ifdef DEBUG_STEPPER
			//printf("****���ٽ׶�***\n");
			//Ϊ��������ͳ�Ʒ���ȷ����ӡ��ʽ���£��磺��ʼ�ٶ�=xxxx  ʱ���=
			printf("�����ٶ�=%6ld  ",current_block->nominal_rate); //ÿ���жϵĲ�������ٶȣ���step/s��
			printf("�ж�ʱ��=%5d  ",TIMER4_nominal);
			add_interrupt_time += TIMER4_nominal;
			printf("ʱ��=%8d\n",add_interrupt_time);
		#endif		
	}
    // If current block is finished, reset pointer //��ǰblockִ���꣬��λָ��
    if (step_events_completed >= current_block->step_event_count) 
	{
      current_block = NULL;
      plan_discard_current_block();
    }   
  } 
 }
}

void st_init(void)
{	
     //����������ע�͵��ĺ�������ʵ���в�δ�õ�������������صĺ�������󣬺���Ҳ����Ӧ˵��	
//	 digipot_init(); //Initialize Digipot Motor Current //��ʼ�����ֵ�λ�������ڲ�����������ϵ��ڵ�����
//   microstep_init(); //Initialize Microstepping Pins  //��ʼ��ϸ�����ţ��������ò������������ϸ��ģʽ��
//Ĭ�ϴ�Ӳ���Ͻ������������ϸ��ģʽ����Ϊ16ϸ�֣�����ͨ�������������ϵ�ģ���λ��������	
	
   //��ʼ������������Ų������������Ϊ�ر�״̬
	 GPIO_InitTypeDef         GPIO_InitStructure;	
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF, ENABLE);	 
	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
   
	//��ʼ��X�Ჽ���������������
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;	
	 GPIO_Init(GPIOF, &GPIO_InitStructure);
	 X_STEP_PIN=INVERT_X_STEP_PIN; //����ų�ʼ��Ϊfalse
   disable_x();	//�ر�X�Ჽ�����ʹ��
  
	 //��ʼ��Y�Ჽ���������������
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;		
	 GPIO_Init(GPIOF, &GPIO_InitStructure);	
	 Y_STEP_PIN=INVERT_Y_STEP_PIN; //����ų�ʼ��Ϊfalse
	 disable_y();	//�ر�Y�Ჽ�����ʹ��

   //��ʼ��Z�Ჽ���������������
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;			
	 GPIO_Init(GPIOF, &GPIO_InitStructure);
	 Z_STEP_PIN=INVERT_Z_STEP_PIN; //����ų�ʼ��Ϊfalse
   disable_z();	//�ر�Z�Ჽ�����ʹ��	 

   //��ʼ��E0�Ჽ���������������
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;			
	 GPIO_Init(GPIOF, &GPIO_InitStructure);
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;			
	 GPIO_Init(GPIOE, &GPIO_InitStructure);
	  E0_STEP_PIN=INVERT_E_STEP_PIN; //����ų�ʼ��Ϊfalse
   disable_e0();	//�ر�e0�Ჽ�����ʹ��	 
   
	 //��ʼ��E1�Ჽ���������������
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;			
	 GPIO_Init(GPIOE, &GPIO_InitStructure);
	 E1_STEP_PIN=INVERT_E_STEP_PIN; //����ų�ʼ��Ϊfalse
   disable_e1();	//�ر�e1�Ჽ�����ʹ��	 


	//��ʼ����λ�������Ų�����λ���صĳ�ʼ״̬��Ϊfalse���ߵ�ƽ������
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTA,PORTCʱ��

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
	//��ʼ����λ����X������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  //��ʼ����λ����Y������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//��ʼ����λ����Z������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5;
 	GPIO_Init(GPIOC, &GPIO_InitStructure); 	 	  
  
	 
  //��ʼ����ʱ��TIM4��
	//��ʱʱ��Ƶ��Ϊ72M/��35+1��=2MHZ 
	//�Զ���װ�ؼĴ������ڵ�ֵΪ0x4000
	TIM4_Int_Init(0x4000,35);
 	
  ENABLE_STEPPER_DRIVER_INTERRUPT();  
  
  enable_endstops(1); //������λ���ؼ�� // Start with endstops active. After homing they can be disabled
 // sei();����ȫ���жϣ�������һ����STM32�п���ȫ���жϵĺ�����������ƺ�û�г���Ҳ����������
}


// Block until all buffered steps are executed
void st_synchronize(void)
{
    while( blocks_queued()) {
    manage_heater();
    manage_inactivity();
		lcd_update();
//	  interface_update(); //��Ļ�������
  }
}

void st_set_position(const long x, const long y, const long z, const long e)
{
  CRITICAL_SECTION_START;
  count_position[X_AXIS] = x;
  count_position[Y_AXIS] = y;
  count_position[Z_AXIS] = z;
  count_position[E_AXIS] = e;
  CRITICAL_SECTION_END;
}

void st_set_e_position(const long e)
{
  CRITICAL_SECTION_START;
  count_position[E_AXIS] = e;
  CRITICAL_SECTION_END;
}

long st_get_position(uint8_t axis)
{
  long count_pos;
  CRITICAL_SECTION_START;
  count_pos = count_position[axis];
  CRITICAL_SECTION_END;
  return count_pos;
}

void finishAndDisableSteppers(void)
{
  st_synchronize(); 
  disable_x(); 
  disable_y(); 
  disable_z(); 
  disable_e0(); 
  disable_e1(); 
}

void quickStop(void)
{
  DISABLE_STEPPER_DRIVER_INTERRUPT();
  while(blocks_queued())
    plan_discard_current_block();
  current_block = NULL;
  ENABLE_STEPPER_DRIVER_INTERRUPT();
}

//����digipot_xxx()�ĺ������Ǻ����ֵ�λ����������������IC�ĵ�����������صĺ��������ڲ��������������
//���õ���ģ���λ�������������������������digipot_xxx()������û�õ������濴������Ż���Щ������
void digipot_init(void) //Initialize Digipot Motor Current //�պ���
{
  /*
	const uint8_t digipot_motor_current[] = DIGIPOT_MOTOR_CURRENT;  
	int i;     
    for(i=0;i<=4;i++) 
      digipot_current(i,digipot_motor_current[i]);
	*/
}

void digipot_current(uint8_t driver, uint8_t current) //�պ���
{
	/*
    const uint8_t digipot_ch[] = DIGIPOT_CHANNELS;
//	printf("%d:%d\r\n",digipot_ch[driver],current);
    digitalPotWrite(digipot_ch[driver], (uint8_t)current);
	*/
}

void digitalPotWrite(uint8_t address, uint8_t value) //�պ���
{
/*
    DIGIPOTSS_PIN=1; // take the SS pin low to select the chip
    SPI1_ReadWriteByte(address); //  send in the address and value via SPI:
    SPI1_ReadWriteByte(value);
    DIGIPOTSS_PIN=0; // take the SS pin high to de-select the chip:
	*/
}

//����һֱ����������ò������������A4988/A4982��ϸ��ģʽ��صĺ�����ʵ���п�ͨ��MS1��MS2��MS3
//�̽�������ϸ��ģʽ����Щ������û�õ������Щ���������ݶ���Ϊ�գ����濴һЩ����Ż��ⲿ������
void microstep_init(void)  //�պ���
{ 
	/*
	int i;
  for(i=0;i<=4;i++) microstep_mode(i,8);
	*/
}

void microstep_ms(uint8_t driver, int8_t ms1, int8_t ms2, int8_t ms3) //�պ���
{
	/*
  if(ms1 > -1) switch(driver)
  {
    case 0:X_MS1_PIN=ms1 ; break;
    case 1:Y_MS1_PIN=ms1 ; break;
    case 2:Z_MS1_PIN=ms1 ; break;
    case 3:E0_MS1_PIN=ms1 ; break;
    case 4:E1_MS1_PIN=ms1 ; break;
	default:  break;
  }
  if(ms2 > -1) 
  switch(driver)
  {
    case 0:X_MS2_PIN=ms2 ; break;
    case 1:Y_MS2_PIN=ms2 ; break;
    case 2:Z_MS2_PIN=ms2 ; break;
    case 3:E0_MS2_PIN=ms2 ; break;
    case 4:E1_MS2_PIN=ms2 ; break;
	default:  break;
  }
    if(ms3 > -1) switch(driver)
  {
    case 0:X_MS3_PIN=ms3 ; break;
    case 1:Y_MS3_PIN=ms3 ; break;
    case 2:Z_MS3_PIN=ms3 ; break;
    case 3:E0_MS3_PIN=ms3 ; break;
    case 4:E1_MS3_PIN=ms3 ; break;
	default:  break;
  }
	*/
}

void microstep_mode(uint8_t driver, uint8_t stepping_mode) //�պ���
{ 
	/*
	switch(driver)
  {
    case 0: subsection_x_value=stepping_mode; break;
    case 1: subsection_y_value=stepping_mode; break;
    case 2: subsection_z_value=stepping_mode; break;
    case 3: subsection_e0_value=stepping_mode; break;
    case 4: subsection_e1_value=stepping_mode; break;
	default:  break;
  }
  switch(stepping_mode)
  {
    case 1: microstep_ms(driver,MICROSTEP1); break;
    case 2: microstep_ms(driver,MICROSTEP2); break;
    case 4: microstep_ms(driver,MICROSTEP4); break;
    case 8: microstep_ms(driver,MICROSTEP8); break;
    case 16: microstep_ms(driver,MICROSTEP16); break;
    case 32: microstep_ms(driver,MICROSTEP32); break;
    case 64: microstep_ms(driver,MICROSTEP64); break;
    case 128: microstep_ms(driver,MICROSTEP128); break;
	default:  break;
  }
	*/
}
void microstep_readings(void)
{
	printf("Motor_Subsection \n");
	printf("X: %d\n",subsection_x_value);
	printf("Y: %d\n",subsection_y_value);
	printf("Z: %d\n",subsection_z_value);
	printf("E0: %d\n",subsection_e0_value);
	printf("E1: %d\n",subsection_e1_value);
}

