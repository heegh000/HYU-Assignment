#include "msp.h"
#include "./Clock.h"
#include <stdio.h>

#define RED     (1 << 0)
#define GREEN   (1 << 1)
#define BLUE    (1 << 2)
#define SIZE    100

/********************************
 SysTick
 ********************************/
void init_SysTick(void) {
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}


void wait_1us(int delay) {
    SysTick->LOAD = 48 * delay;
    SysTick->VAL = 0;
    while((SysTick->CTRL & 0x00010000) == 0) {};
}

/********************************
 Timer
 ********************************/
void (*timer_A2_task) (void);
void init_timer_A2(void (*task)(void), uint16_t period) {
    timer_A2_task = task;
    TIMER_A2->CTL = 0x0280;
    TIMER_A2->CCTL[0] = 0x0010;
    TIMER_A2->CCR[0] = (period - 1);
    TIMER_A2->EX0 = 0x0005;
    NVIC->IP[3] = (NVIC->IP[3] & 0xFFFFFF00) | 0x00000040;
    NVIC->ISER[0] = 0x00001000;
    TIMER_A2->CTL |= 0x0014;
}

void TA2_0_IRQHandler(void) {
    TIMER_A2->CCTL[0] &= ~0x0001;
    (*timer_A2_task)();
}

/********************************
 LED
 ********************************/
// LED2 설정
void init_led(void) {
    // Setup P2 0~2 bit & P2 0 bit as GPIO
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;
    // Setup P2 0~2 bit & P2 0 bit as OUTPUT
    P2->DIR |= 0x07;
    // Turn off all LEDs
    P2->OUT &= ~0x07;
}

void turn_on_led(int color) {
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}

void turn_off_led(void) {
    P2->OUT &= ~0x07;
}

/********************************
 IR SENSOR
 ********************************/

// IR SENSOR 설정
void init_IR_sensor(void) {
    // GPIO
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    //OUPUT
    P5->DIR |= 0x08;
    // Turn off even LEDs
    P5->OUT &= ~0x08;

    // GPIO
    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    // OUTPUT
    P9->DIR |= 0x04;
    //Turn off odd LEDs
    P9->OUT &= ~0x04;

    // GPIO
    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    // INPUT
    P7->DIR &= ~0xFF;
}

void turn_on_ir_led(void) {
    P5->OUT |= 0x08;
    P9->OUT |= 0x04;
}

void charge_capacitor(void) {
    P7->DIR = 0xFF;
    P7->OUT = 0xFF;
    wait_1us(10);
}

void convert_to_input(void) {
    P7->DIR = 0x00;
    Clock_Delay1ms(1);
}

void turn_off_ir_led(void) {
    P5->OUT &= ~0x08;
    P9->OUT &= ~0x04;
}

/********************************
 SWITCH
 ********************************/

void init_switch(void) {
    // Setup Switch as GPIO
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;

    // Setup Switch as INPUT
    P1->DIR &= ~0x12;

    // Enalbe pull resistors
    P1->REN |= 0x12;

    // Now pull-up
    P1->OUT |= 0x12;
}

//  left: switch[0] = P1->IN & 0x10;
//  right: switch[1] = P1->IN & 0x02;

/********************************
 PWM
 ********************************/

void init_34_PWM(uint16_t period, uint16_t duty3, uint16_t duty4) {
    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;

    TIMER_A0->CCTL[0] = 0x800;
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;
}

void set_PWM_duty3(uint16_t duty3) {
    TIMER_A0->CCR[3] = duty3;
}

void set_PWM_duty4(uint16_t duty4) {
    TIMER_A0->CCR[4] = duty4;
}


/********************************
 Motor
 ********************************/

void init_motor(void) {
    // Setup DIRR & DIRL as GPIO
    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    // Setup DIRR & DIRL as OUTPUT
    P5->DIR |= 0x30;
    // OUTPUT Low
    P5->OUT &= ~0x30;

    // Setup PWMR & PWML as GPIO
    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    // Setup PWMR & PWML as OUTPUT
    P2->DIR |= 0xC0;
    // OUTPUT Low
    P2->OUT &= ~0xC0;

    // Setup nSLPR & nSLPL as GPIO
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    // Setup nSLPR & nSLPL as OUTPUT
    P3->DIR |= 0xC0;
    // OUTPUT Low
    P3->OUT &= ~0xC0;

    init_34_PWM(15000, 0, 0);
}

void set_speed(uint16_t left_duty, uint16_t right_duty) {
    P3->OUT |= 0xC0;
    set_PWM_duty3(right_duty);
    set_PWM_duty4(left_duty);
}

void set_left_forward() {
    P5->OUT &= ~0x10;
}
void set_left_backward() {
    P5->OUT |= 0x10;
}

void set_right_forward() {
    P5->OUT &= ~0x20;
}
void set_right_backward() {
    P5->OUT |= 0x20;
}

void stop(void) {
    // PWMR & PWML (EN) = 0
    P2->OUT &= ~0xC0;
}


void init_lontea(void) {
    init_led();
    init_IR_sensor();
    init_SysTick();
    init_switch();
    init_motor();
}


enum direction {
    start,
    straight,
    back,
    left,
    right,
    correction,
    end,
    test
};

int sensor[8];

void read_sensor(void) {
    turn_on_ir_led();
    charge_capacitor();
    convert_to_input();
    int i;
    for(i = 0; i < 8; i++) {
        sensor[i] = P7->IN & (1 << i);
    }

    turn_off_ir_led();
}

void move(int dir, int lspeed, int rspeed, int t) {
    if(dir == straight) {
        set_left_forward();
        set_right_forward();
    }
    else if(dir == left) {
        set_left_backward();
        set_right_forward();
    }
    else if(dir == right) {
        set_left_forward();
        set_right_backward();
    }
    else if(dir == correction) {
        set_left_backward();
        set_right_backward();
    }

    set_speed(lspeed, rspeed);
    if(t != 0) {
        Clock_Delay1ms(t);
    }
}
/********************************
  LSRB
  ********************************/
int branch_list[SIZE];
int branch_top = 0;

int optimal_route[SIZE];
int optimal_top = 0;

int is_empty_optimal() {
   if(optimal_top == 0) {
      return 1;
   }
   else {
      return 0;
   }
}

int is_full_optimal() {
   if(optimal_top == SIZE) {
      return 1;
   }
   else {
      return 0;
   }
}

void push_optimal(int dir) {
   if(is_full_optimal()) {
      return;
   }
   optimal_route[optimal_top] = dir;
   optimal_top += 1;
}

void pop_optimal() {
   if(is_empty_optimal()) {
      return;
   }
   optimal_top--;
}

int peek_optimal() {
   if(is_empty_optimal()) {
      return -1;
   }
   return optimal_route[optimal_top-1];
}

void run_lsrb() {
    int is_back = 0;
    int now = 0;
    int dir = 0;

    while(now < branch_top) {
        dir = branch_list[now++];
        if(dir == back) {
            is_back = 1;
        }

        if(is_back) {
            if(peek_optimal() == left) {
                if(dir == left) {
                    pop_optimal();
                    push_optimal(straight);
                    is_back = 0;
                }
                else if(dir == straight) {
                    pop_optimal();
                    push_optimal(right);
                    is_back = 0;
                }
                else if(dir == right) {
                    pop_optimal();
                    is_back = 1;
                }
            }
            else if(peek_optimal() == straight) {
                if(dir == left) {
                    pop_optimal();
                    push_optimal(right);
                    is_back = 0;
                }
                else if(dir == straight) {
                    pop_optimal();
                    is_back = 1;
                }
            }
            else if(peek_optimal() == right) {
                if(dir == left) {
                    pop_optimal();
                    is_back = 1;
                }
            }
        }
        else if(dir != back) {
            push_optimal(dir);
        }
    }
}

void control_lontea(int phase) {
    int speed[2] = {3005, 3180};
    int loop_delay = 10;
    int mode = start;

    int is_branch = 0;
    int branch_cnt = 0;
    int now = 0;

    while(1) {
        if(mode == start) {
            move(straight, 0, 0, 0);
            while(1) {
                read_sensor();
                if(!sensor[7] && sensor[6] && sensor[5] && sensor[4] && sensor[3] && sensor[2] && sensor[1] && !sensor[0]) {
                    mode = straight;
                    move(straight, speed[0], speed[1], 250);
                    break;
                }
                Clock_Delay1ms(loop_delay * 20);
            }
        }
        else if(mode == end) {
            move(straight, 0, 0, 0);
            turn_on_led(RED);
            break;
        }
        // 직진모드
        else if(mode == straight) {
            move(straight, speed[0], speed[1], 0);
            while(1) {
                move(straight, speed[0], speed[1], 0);
                read_sensor();
                turn_off_led();
                branch_cnt--;

                // 종료 인식
                if(sensor[6] && sensor[5] && sensor[4] && sensor[3] && sensor[2] && sensor[1]) {
                    if(!sensor[7] || !sensor[0]) {
                        mode = end;
                        break;
                    }
                }
                // 회전 인식
                if(mode == straight && ((sensor[7] && sensor[6]) || (sensor[1] && sensor[0]))) {
                    is_branch = 0;
                    if(sensor[7] && sensor[6]) {
                        mode = left;
                    }
                    else {
                        mode = right;
                    }

                    Clock_Delay1ms(50);
                    read_sensor();
                    if(sensor[7] && sensor[6]) {
                        mode = left;
                    }
                    else {
                        mode = right;
                    }

                    // 분기점 확인
                    if(sensor[7] && sensor[6] && sensor[5] && sensor[4] && sensor[3] && sensor[2] && sensor[1] && sensor[0]) {
                        is_branch = 1;
                    }

                    // 흰색을 만날때까지 감
                    while(1) {
                        read_sensor();
                        if(mode == left && !sensor[7] && !sensor[6] ) {
                            break;
                        }
                        if(mode == right && !sensor[1] && !sensor[0]) {
                            break;
                        }
                        if(!sensor[4] && !sensor[3]) {
                            break;
                        }
                        Clock_Delay1ms(loop_delay);
                    }

                    // 오른쪽보다 직진이 우선순위 더 높음
                    Clock_Delay1ms(40);
                    read_sensor();
                    if(sensor[4] || sensor[3]) {
                        if(mode == right) {
                            mode = straight;
                        }
                        is_branch = 1;
                    }

                    if(is_branch && branch_cnt <= 0) {
                        if(phase == 1) {
                            branch_list[branch_top++] = mode;
                            turn_on_led(BLUE);
                            branch_cnt = 5;
                        }
                        else if(phase == 2) {
                            mode = optimal_route[now++];
                            turn_on_led(BLUE);
                        }
                    }
                }
                else if(mode == straight && (!sensor[4] || !sensor[3])) {
                    Clock_Delay1ms(15);
                    read_sensor();

                    // 뒤로 돌기
                    if(!sensor[7] && !sensor[6] && !sensor[5] && !sensor[4] && !sensor[3] && !sensor[2] && !sensor[1] && !sensor[0]) {
                        if(phase == 1) {
                            mode = back;
                            branch_list[branch_top++] = mode;
                            turn_on_led(GREEN);
                        }
                    }
                    else if(!sensor[7] && !sensor[0]){
                        // 보정
                        while(!(sensor[4] && sensor[3]) && mode == straight) {
                            read_sensor();
                            if(sensor[4] && sensor[3]) {
                                move(straight, speed[0], speed[1], 0);
                                break;
                            }
                            else if(!sensor[4] && sensor[3]) {
                                move(straight, speed[0] / 2, speed[1] / 2, 50);
                                move(straight, 0, 0, 100);

                                move(correction, speed[0] - 1800, speed[1] - 800, 200);

                            }
                            else if(sensor[4] && !sensor[3]){
                                move(straight, speed[0] / 2, speed[1] / 2, 50);
                                move(straight, 0, 0, 100);

                                move(correction, speed[0] - 800, speed[1] - 1800, 200);

                            }
                            Clock_Delay1ms(loop_delay);
                        }

                    }
                }

                if(mode != straight) {
                    break;
                }

                Clock_Delay1ms(loop_delay);
            }
        }
        else if(mode == left || mode == right) {
            move(mode, speed[0], speed[1], 400);
            while(1) {
                read_sensor();
                if(!sensor[6] && !sensor[5] && sensor[4] && sensor[3] && !sensor[2] && !sensor[1]) {
                    Clock_Delay1ms(10);
                    break;
                }
                if(mode == right && !sensor[7] && sensor[6] && sensor[5] && sensor[4] && sensor[3] && sensor[2] && sensor[1] && !sensor[0]) {
                    mode = end;
                    break;
                }
                Clock_Delay1ms(loop_delay);
            }
            mode = straight;
        }
        else if(mode == back) {
            move(left, speed[0], speed[1], 700);
            while(1) {
                read_sensor();
                if(!sensor[6] && !sensor[5] && sensor[4] && sensor[3] && !sensor[2] && !sensor[1]) {
                    break;
                }
                Clock_Delay1ms(loop_delay);
            }
            mode = straight;
        }
    }
}

int main(void) {
    Clock_Init48MHz();
    init_lontea();

    control_lontea(1);
    run_lsrb();

    while(1) {
        int sw = P1->IN & 0x02;
        if(!sw) {
            break;
        }
        Clock_Delay1ms(200);
    }

    control_lontea(2);
}
