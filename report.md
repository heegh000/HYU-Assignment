
# Team

2018009234 한관희

# Hardware Control Function

## 1. init

![lt0](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/8ac926be-eb49-43f1-9be8-063c2a23b261)

저의 TI-RSLK MAX 기기의 이름은 론티(lontea)입니다. 평소 즐겨 마시던 음료인 실론티 ZERO와 색 조합이 비슷하여서 지은 이름입니다.

init_lontea 함수에서, LED, IR 센서, SystTcik, switch, motor를 초기화하는 함수들을 호출합니다. 각 함수는 실습 수업에서 배운 내용과 동일하게 구현하였습니다.

## 2. move

![lt1](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/cccf9b6d-9fc2-45b9-8642-7178fc57c1c2)


실습 수업에서 배운 내용과 동일하게, PWM을 이용한 motor 코드를 함수로 구현하였습니다. 

![lt2](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/b932cd64-ced2-4671-af36-99da2bab562d)
![lt3](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/446710d2-5f59-4775-bec3-3c9e0a2a5184)


move 함수에서 위의 함수들을 호출하여, 기기의 모터 상태를 제어하고 있습니다.

### 3. IR sensor

![lt4](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/167b5618-1085-43e3-b0c3-19b9e79f694d)

실습 수업에서 배운 내용을 참고하여, IR 센서와 관련된 제어 코드들을 함수로 구현하였습니다.

![lt5](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/9df88d68-a67b-4cbc-8dc5-691ab3943e55)

read_sensor 함수에서, 해당 함수들을 호출하여 전역 변수로 선언된 sensor 변수에 현재 IR 센서의 값을 읽어옵니다.

# Line Tracing

## 1. control_lontea

![lt6](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/ac91f363-9e3a-47eb-8442-52f6b7c13838)

control_lontea 함수는는 move 함수와 read_sensor 함수를 호출하여 line tracing을 수행하는 함수입니다. 

기기는 매 상황마다 mode를 가지고 있으며, 현재 mode에 맞는 행동을 반복적으로 수행합니다.

주기적으로 IR 센서를 읽고, 현재 상태를 파악하여, mode를 변형해야 한다면 변경합니다.

각 mode와 mode에 맞는 행동에 대해서는 아래 자세히 작성하도록 하겠습니다.

## 2. start mode

![lt7](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/017fdb6b-8c4f-477f-b2f0-baba8886e661)

*start* mode는 단순합니다.

motor를 움직이지 않는 상태로 바꿉니다.

6번, 5번, 4번, 3번, 2번, 1번 센서가 Black일때, mode를 *straight*로 변경합니다.

## 3. end mode

![lt8](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/f4d8f244-7a02-4779-8dae-56b7ee3a1e78)

*end* mode는 더 단순합니다.

기기를 멈추고, 반복문을 탈출하여, control_lontea 함수에서 return합니다.

## 4. straight mode

*straight* mode는 굉장히 복잡합니다.

*straight* mode에서는 IR 센서의 상태에 따라 *end*, *left*, *right*, *back* mode로 전환하기 때문입니다.

![lt9](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/d9ab7af7-b3ec-463b-beba-c7f6689f32f8)

먼저 매 loop 마다 IR 센서의 상태를 읽어오고 해야 할 행동을 수행하고, mode를 설정합니다.

- line 424 ~ 429

종료 인식과 관련된 코드입니다.

6번, 5번, 4번, 3번, 2번, 1번 센서가 Black이고, 7번 혹은 0번 센서가 White인 경우 *end*로 판단합니다.

![lt10](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/6a909159-36cf-4b97-8701-74a01e02d195)

- line 430 ~ 490

좌회전, 우회전을 인식하는 코드입니다.

좌회전, 우회전을 판단하기 위해 각각 7번과 6번, 1번과 0번 센서를 이용하였습니다. 

7번과 6번 센서, 1번과 0번 센서중 한 쪽이라도 Black이 된다면, 회전을 감지한 것입니다. 

저는 Phase 1에서 Left Hand on Wall을 채택했기때문에 좌회전 > 직진 > 우회전의 우선순위를 가지고 있습니다.

- line 440 ~ 447

기기가 비스듬이 간 경우도 존재하기 때문에 우회전으로 판단했지만, T자형 코너일 수도 있습니다.

좌회전이 우회전보다 우선순위가 높기 때문에, T자형 코너에서는 좌회전을 해야 합니다. 

따라서 약간의 term 주어서 50ms 후에 다시 판단합니다. 

7번, 6번, 5번, 4번, 3번, 2번, 1번, 0번 센서가 모두 Black인 경우, 즉 T자형 코너인 경우 **분기(branch)** 로 판단합니다.

- line 455 ~ 467

그 후, 센서가 White를 만날 때까지 직진합니다.

이렇게 구현한 이유는, 센서의 위치가 바퀴의 위치와 일치하지 않아서, 조금 더 전진 후 회전해야 하기 떄문입니다.

- line 470 ~ 477

이후 약간 더 직진하여 앞에 길이 있는지 확인합니다.

이렇게 구현한 이유는, ㅏ자형 코더인 경우를 감지하기 위해서 입니다.

직진이 우회전보다 우선순위가 높기 때문에, ㅏ자형 코너에서는 직진을 해야 합니다.

ㅏ자형 코너라고 판단한 경우에도 **분기(branch)** 로 판단합니다.


![lt11](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/55b2b3ad-6683-4d72-9eea-a852ff85a309)

- line 491 ~ 537

180도 회전과 correction을 감지하는 부분입니다.

4번 혹은 3번 센서가 하나라도 White가 된다면, 이는 180도 회전을 인식하거나 보정을 해야하는 경우입니다.

- line 492 ~ 493

180도 회전을 해야 하는지, 보정을 해야 하는지 확실하게 알기 위해서 회전 인식때와 마찬가지로 약간의 term을 주어서 15ms 후에 다시 확인합니다.

- line 496 ~ 502

다시 확인했을 때, 모든 센서가 White라면 180도 회전을 해야 한다고 판단하여 mode를 *back*으로 변경합니다.

180도 회전을 해야 하는 경우에도 **분기(branch)** 로 판단합니다.

- line 503 ~ 529

만약 모든 센서가 White가 아니라면, 보정을 해야 한다고 판단하여 보정 코드를 수행합니다. 

보정 코드는 전진과 후진을 반복하는데, 후진할 때에는 나간 센서에 방향에 맞춰 두 바퀴의 속도를 다르게 조정합니다. 

보정 코드는 4번, 3번 센서가 모두 Black일 때까지 수행됩니다.

## 5. left, right mode

![lt12](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/3ce12606-7430-4ec4-a9a1-026c20e06f6c)

*left*, *right* mode는 회전을 수행합니다.

- line 539

현재 방향으로 400ms 정도 고정적으로 돌기 시작합니다.

- line 540 ~ 551

6번, 5번, 2번, 1번 센서가 White이고, 4번 3번 센서가 Black이 될 때까지 회전합니다.

회전이 끝나면 mode를 *striaght*로 바꿉니다.

## 6. back mode

![lt13](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/e5e6db35-32e0-41e9-ab1e-bc4fb831d71b)

*back* mode는 *left* mode, *right* mode와 유사합니다.

# Phase 1. Memorize

## 1. main function

![lt14](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/3878cbe8-c625-4f4b-88aa-d0accfa4f97c)

main 함수에서 control_lontea(1)을 호출하여, Phase 1을 수행합니다.

## 2. Left Hand on Wall

저의 Phase 1에서, 기기는 Left Hand on Wall을 통해 미로를 탈출합니다.

기기는 좌회전 > 직진 > 우회전의 우선순위로 움직입니다.

Left Hand on Wall를 수행하기 위한 자세한 구현은 위의 Line Tracing의 *straight* mode에서 설명하였습니다.

## 3. Branch

저는 분기와 코너를 구별하고 있습니다.

코너는 기기가 갈 수 있는 길이 하나인 경우이며, 구불구불한 직선과 같습니다.

분기는 기기가 갈 수 있는 길이 여러 개인 경우로, T자형, ㅏ자형, 180도 회전이 분기로 판단됩니다.

분기 인식에 대한 자세한 설명은 Line Tracing에 작성되어 있습니다. 

![lt15](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/7de063a7-22bc-460b-88c2-0b6c0ec541e4)
![lt16](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/37d28c3a-17e7-423f-add1-b3a3b1d255da)


분기로 인식되는 경우 Phase 1에서는 현재 기기가 수행한 방향을 branch_list라는 배열에 추가합니다.

line 479~484는 *straight* mode에서 T자형, ㅏ자형 분기를 인식했을 경우이고, line 496 ~ 502는 180도 회전을 인식했을 때 수행되는 코드입니다.

추가적으로, 과제를 하는 과정에서 분기가 여러번 인식되는 문제가 있었습니다.

보정 코드가 후진을 통해 이루어지기 때문에 발생한 문제였고, 이를 해결하기 위해 bracnh_cnt라는 변수를 만들어서 보정 코드가 실행된 후 while문이 일정이상 돌지 않으면, 분기 인식을 수행하지 않게 하였습니다.

# Phase 2. Escape

## 1. main function

![lt14](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/9bfddbee-5667-42e8-a174-649176de9ed8)

LSRB 알고리즘을 수행한 후, 오른쪽 스위치가 눌릴 때까지 기다립니다.

오른쪽 스위치가 눌리면 control_lonea(2)를 호출하여, Phase 2를 수행합니다. 


## 2. LSRB Algorithm

Left Hand on Wall 경로 최적화를 위하여, LSRB 알고리즘을 사용했습니다.

LSRB 알고리즘이란, Left Hand on Wall 가지 않아도 될 분기를 제거하는 알고리즘입니다.

예를 들어, T자형 분기에서 Left Hand on Wall 방식은 왼쪽부터 가지만, 왼쪽이 막다른 길인 경우 오른쪽부터 가는 것이 옳은 선택일 것입니다.

LSRB 알고리즘을 통해, 아래와 같이 좌회전(L), 직진(S), 우회전(R), 180도 회전(B) 3개를 하나의 행동으로 치환할 수 있습니다.

> LBR = B
> 
> LBS = R
> 
> RBL = B
> 
> SBL = R
> 
> SBS = B
> 
> LBL = S
> 


![lt17](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/d002d4e0-ff60-4a20-886f-d47a19546f8d)

저는 Phase 1에서 생성된 저장된 경로(***branch_list***)를 최적화하기 위하여, 배열로 구현된 stack인 ***optimal_route***에 push와 pop 연산으로 LSRB 알고리즘을 구현하였습니다.

![lt18](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/9dbfb753-9d33-4ec4-aec3-0278d9bdff5c)

위에서 작성한 규칙대로 ***optimal_route***를 완성합니다.

## 3. Branch

Phase 1과 마찬가지로 분기를 탐지합니다.

분기라고 탐지한 경우, Phase 1에서는 Left Hand on Wall을 따른 후 경로를 저장했지만 Phase 2에서는 optimal_route의 top으로 가야할 방향을 설정합니다. 

![lt15](https://github.com/heegh000/HYU-Embedded-System-Design/assets/108382134/7de063a7-22bc-460b-88c2-0b6c0ec541e4)

line 485 ~ 489는  T자형, ㅏ자형 분기를 인식했을 경우 최적화된 경로로 mode를 세팅하는 코드입니다. 

LSRB 알고리즘을 이용하면 180도 회전을 없앨 수 있으므로, Phase 2에서 180도 회전과 관련된 코드는 없습니다.

Phase 1과 Phase 2에서 기기의 기본 행동은 동일하지만, 분기에서는 다르게 행동하게 구현하였습니다

Phase 1은 분기에서의 현재 결정을 저장하고, LSRB 알고리즘은 저장된 경로를 최적화 하고, Phase 2는 분기에서 최적화된 경로를 사용하는 것이 저의 구현입니다.
