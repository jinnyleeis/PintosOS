

# Pintos Project 3: Threads

담당 교수 : 김영재
조 / 조원 : 이지인
개발 기간 : 11월 16~11월 24일


## 개발 목표
해당 프로젝트에서 구현할 내용을 간략히 서술.
1. Alarm Clock 현재 스레드는 busy wating으로 계속적으로 thread_yield를 호출하고 있는 것이지, 실제적으로 스레드가 sleep하고 있는 것이라 볼 수 없다. 때문에 스레드가 실제로 틱만큼 sleep하고, 해당 시간이 지나면, wake할 수 있도록 해야한다. 
2. Priority Scheduling 현재 핀토스의 스케줄링 알고리즘은 round robin 알고리즘이다. 이를, 우선순위에 따라 스레드가 선택될 수 있도록, 우선 순위 알고리즘을 구현하고, 에이징을 구현한다. 
3. Advanced Scheduler (추가구현을 한 경우) recent cpu와 nice 값을 기반으로 우선순위를 계산을 하여, thread의 우선운위를 조정하는 mlfq를 구현한다. 

## 개발 범위 및 내용
### 개발 범위
아래 각 항목 개발의 필요성 또는 개발 시 기대되는 결과를 간략히 서술
#### Alarm Clock
개발 목표에서도 이야기하였듯이, 기존의 Pintos 구현에서 timer_sleep()은 스레드가 busy waiting을 통해, 비효율적으로, sleep을 처리하였다. 이를 효율적인 슬립 메커니즘으로 변경할 수 있도록 해서, 스레드가 바쁜 대기 없이 지정된 틱 수만큼 슬립하고, 슬립 시간이 경과하면 정확하게 깨울 수 있게 하는 것이 목표다. 
#### Priority Scheduling
개발 목표에서도 이야기하였듯이, 핀토스  기본 스케줄러는 라운드 로빈 알고리즘을 사용한다. 핀토스의 기본 스케줄링 알고리즘을 우선순위 알고리즘으로 대체할 수 있도록 해서, 스케줄러가 스레드의 우선순위에 따라 실행할 스레드를 선택할 수 있게 한다. 이 높은 우선순위의 스레드가 준비 상태가 되면, 낮은 우선순위의 실행 중인 스레드를 선점
#### Advanced Scheduler (추가구현을 한 경우)
 MLFQS 스케줄러는, recent_cpu(최근 cpu 사용량)와 nice 값을 기반으로 우선순위를 조정하여, 스케줄링을 fairness하게 가능하게 한다. 즉, 핀토스 시스템에서 이 두 값을 기반으로 우선순위가 계산되어, 이 값을 주기적으로 업데이트해, 스케줄링을 진행하는 것이다. 

## 개발 내용
아래 항목의 내용만 서술
### 1) Blocked 상태의 스레드를 어떻게 깨울 수 있는지 서술.
(1 )스레드가 timer_sleep()을 호출하여 슬립을 요청하면, wake_up_time을 설정하고 슬립 리스트에 삽입한 후, thread_block()을 통해 Blocked 상태로 전환한다.
(2) 타이머 인터럽트가 발생할 때마다 timer_interrupt()가 호출되고, 이 안에서 thread_awake(ticks);가 실행된다.
(3) thread_awake() 함수를 통해, 슬립 리스트를 순회해서, 깨어날 시간이 된 스레드들을 찾아 thread_unblock()을 호출해서, Ready 상태로 전환한다.
![image](https://github.com/user-attachments/assets/41e0d6f1-89d4-4280-8331-88ccbb64f554)
![image](https://github.com/user-attachments/assets/b8687914-93e2-4bd7-ab0b-6f6a15beb70b)

### 2) Ready list에 running thread보다 높은 priority를 가진 thread가 들어올 경우 priority scheduling에 따르면 어떻게 해야하는지 서술.
이러한 상황이 되었을 경우, running thread보다 우선순위가 높은 새로 생성된 thread가 ‘선점’을 한다. 이를 위해서,  현재 실행 중인 스레드와 Ready 리스트에서 가장 높은 우선순위를 가진 스레드의 우선순위를 비교하하는 과정을 거쳐서, 현재 스레드의 우선 순위가 더 낮을 경우,  thread_yield()를 호출하여 CPU를 yield하도록 하는 로직(함수)이 필요하다. 

### 3) Advanced Scheduler에서 priority 계산에 필요한 각 요소를 서술. (추가구현을 한 경우)

![image](https://github.com/user-attachments/assets/2b1e8366-f2ca-4543-9efe-bb03e3b05791)

advanced scheduler에선, 우선 순위 계산에 크게, ‘최대 우선순위 값’+스레드의 최근 cpu 사용량, 스레드의 nice값이 필요하다. 또한, 핀토스에서는 부동 소수점 연산을 지원하지 않으므로, 고정 소수점 연산을 통한 별도의 매크로와 함수를 사용하여, 이러한 priority 계산을 수행하기 위한 연산을 진행한다. 










## 추진 일정 및 개발 방법
### 추진 일정
11월 16일–17일 : 기초 세팅 진행
11월 18일 : 고정 소수점 연산을 위한 enum, 함수, 매크로 구현 /스케줄링 모드 - priority mlfq set 설정 함수 구현  /alarm clock, priority scheduling 구현
11월 23,24일 : mlfq 추가 구현 진행

![image](https://github.com/user-attachments/assets/e67d3607-fae7-4f4f-b80a-9673578d2a1b)




### 개발 방법
#### II. B.의 개발 내용을 구현하기 위해 각각에 대해 다음 사항들을 포함하여 설명
##### 수정해야하는 소스코드

devices/timer.c
threads/synch.c
threads/init.c
threads/thread.h
threads/thread.c

##### 수정하거나 추가해야 하는 자료구조
thread.h의 struct thread 자료 구조 내에 수정 사항들 
 alarm clock 구현을 위해서, wake_up_time 변수를 추가
MLFQS를 위해 nice와 recent_cpu를 추가
sleep_list 자료구조 추가 
sleep 중의 스레드를 관리하는 전역 리스트 - wake_up_time 순으로 정렬
scheuling mode 관리를 위한 enum 자료구조 추가
부동 소수점 연산 종류 enum 자료 구조 추가 
fixed_point_t: 고정소수점 연산을 위해 정의
thread.c에 매크로로, fixed_point_operation() 함수를 호출하여 연산 매크로를 구현함 

##### 수정하거나 추가해야 하는 함수 - alarm clock 관련해서 
void thread_sleep(int64_t ticks) : timer sleep함수에서 호출해서  current ticks를 통해, wake up time을 설정하고, thread를 블록시킬 함수 추가 
void thread_awake(int64_t current_ticks) : 슬립 시간이 경과한 스레드를 깨울 함수 추가 
bool cmp_wake_up_time : sleep_list를 wake_up_time에 따라 정렬하기 위한 비교 기준으로 사용하기 위해 추가
timer_interrupt() 수정 : busy wating -> thread_awake(ticks)를 호출해서 스레드를 효율적으로 꺠울 수 있도록 수정 
수정하거나 추가해야 하는 함수 - priority scheduling 관련해서 
bool cmp_priority 함수 추가 : ready_list를 스레드 우선순위에 따라 정렬하기 위해 추가 
void test_max_priority(void) 추가 : 현재 running thread가 yield해야 하는지 검사를 위해 추가
void thread_unblock (struct thread *); 함수 수정 : 우선순위에 따라 ready_list에 추가할 수 있도록 수정
void sema_down(struct semaphore *sema) 수정 : wait list에 추가될때도 우선순위에 정렬되도록 
void sema_up(struct semaphore *sema) 수정 : wait list에 가장 높은 우선순위의 스레드가 unblock되도록
 thread_aging(void) 함수 추가 
thread_start, thread_yield 수정
thread_set_priority, thread_get_prirority 수정
init_thread 함수, thread_start 함수 수정

##### 수정하거나 추가해야하는 함수 - ADVANCED SCHEULDER 관련해서 
fixed_point_t fixed_point_operation(fixed_point_op_t op, fixed_point_t A, int B, fixed_point_t C)  : 정된 연산 타입에 따라 필요한 여러 고정소수점 연산을 수행할 수 있도록 구현
void calculate_priority(struct thread *t, void *aux UNUSED) : thread의 우선순위를 mlfqs의 우선순위 계산 규칙에따라 계산할 수 있도록 구현
void calculate_recent_cpu(struct thread *t, void *aux UNUSED) : recent cpu 값을 업데이트할 수 있도록 추가
void calculate_load_avg(void): load_avg 값을  재계산)하도록 추가
static void update_recent_cpu(struct thread *t)-> 매 틱마다 실행 중인 스레드의 recent_cpu를 업데이트(증가)시킬 수 있도록 추가 
static void update_load_avg_and_recent_cpu(void) ->  매초마다, 모든 스레드의 recent_cpu와 load_avg를 재계산할 수 있도록 호출하기 위해 추가
static void update_priority_and_yield(void) -> 매 time_slice 매크로에 해당하는 틱마다, 모든 스레드의 우선순위를 재계산하도록 추가 
void thread_set_scheduling_mode(bool mlfqs) 함수  추가 : 스케줄링 모드가 우선순위 모드 한가지만 있는 것이 아니므로, 각각의 모드대로 알맞은 동작을 수행할 수 있도록 해당 함수 추가 
thread_set_nice (int nice UNUSED) , thread_get_nice (int nice UNUSED) 


## 연구 결과
### Flow Chart
II. B. 개발 내용의 각 항목에 대하여 Flow Chart 작성
(추가구현에 대해서는 flow chart를 작성하지 않아도 됨)

<img width="451" alt="image" src="https://github.com/user-attachments/assets/78b8946b-50c7-4d11-ac6c-eaff53f47930" />











2. priority scheduling에 대해서는, thread_create, thread_awake, sema_up에서 thread_unblock 호출 시, 어떻게 우선순위에 맞추어 스케줄링이 되는지를 플로우차트로 그려보았다. (여기서, 현재 thread priority보다, 높은 우선 순위를 가진 스레드가 들어올 경우 어떻게 해야하는지도 포함되어 있습니다. )
<img width="373" alt="image" src="https://github.com/user-attachments/assets/33233c7a-bb2e-40b4-9124-8f7358906a89" />


## 제작 내용
### II. B. 개발 내용의 각 항목에 대하여 실질적으로 구현한 코드의 관점에서 작성 (구현 내용, 알고리즘 등을 명확히 서술할 것)
구현에 있어 Pintos에 내장된 라이브러리나 자체 제작한 함수를 사용한 경우 이에 대해서도 설명
#### alarm clock
void thread_sleep(int64_t ticks) : timer sleep함수에서 호출해서  current ticks를 통해, wake up time을 설정하고, thread를 블록시킬 함수 추가 
![image](https://github.com/user-attachments/assets/465f2416-8e19-4b40-b11c-9432875ef652)

현재 틱에 ticks를 더하여 해당 thread의 wake_up_time을 계산해서 할당했다. 또한, 이렇게 계산된 wake_up_time을 기준으로 sleep lis에 삽입될수록 하기 위해, list_insert_ordered()와 cmp_wake_up_time()을 사용하여 스레드를 sleep_list에 삽입하였다. 또한, sleep될 수 있도록, thread_block()를 호출하여 스레드를 블록했다. 

![image](https://github.com/user-attachments/assets/cfdd5292-810d-4847-9ab3-71cb2baba125)

timer_sleep에서 알맞은 absolute한 wake up time을 계산해서 이를  thread_sleep을 호출할 때 넘겨주어서, busy waiting이 아닌, 실제로 스레드를 슬립하도록 수정하였다. 
void thread_awake(int64_t current_ticks) : 슬립 시간이 경과한 스레드를 깨울 함수 추가 

![image](https://github.com/user-attachments/assets/3391a10c-9e36-4dff-9070-d7d43e3608cb)

각 스레드에 대해 wake_up_time <= current_ticks인지 확인한 후에, 만약, 현재 틱보다 wake up time이 작거나  같으면, sleep list에서 이를 팝하고 언블록하도록 했다. 만약, 첫번째 요소가 해당 조건을 만족하지 않으면, 이는 wkae up time에 대해 오름차순으로 정렬되어 있는 것이므로, 이후의 wake up time은 다 current ticks보다 크게 될 것이다. 떄문에, 더 이상 검사할 필요없이, break문으로 처리하였다. 그리고, timer_interrupt 함수에서, : busy wating 로직을 빼고, 자고 있는 스레드를 틱이 지났으면, 꺠울 수 있도록 매 틱마다 thread_awake(ticks)를 호출해서 검사가 가능하도록 했다.

<img width="451" alt="image" src="https://github.com/user-attachments/assets/ce50cd63-0b9a-4bf2-827f-91ca6430ff61" />

bool cmp_wake_up_time : sleep_list를 wake_up_time에 따라 정렬하기 위한 비교 기준으로 사용하기 위해 아래와 같은 함수를 추가하였다. 여기서, a의 wake up time이 더 작을시, true 아닐시, false를 반환하도록 구현하였다. 
![image](https://github.com/user-attachments/assets/d51ffec6-8835-43e6-9415-a63b162d742e)


#### priority schedulig
bool cmp_priority 함수 추가 : ready_list를 스레드 우선순위에 따라 정렬하기 위해 추가 

![image](https://github.com/user-attachments/assets/7dddb3d6-b160-4e22-b645-c36c142da1da)

이것도, ready_list를 스레드 우선순위에 따라 정렬하기 위한 비교자 함수로 사용하기 위해 구현하였는데, 우선순위는 큰게 더 높은 것이므로, a 스레드의 우선순위가 더 크면, true를 반환해, a가 더 높은 우선순위를 가지는 스레드임을 확인할 수 있도록 구현했다. 




void test_max_priority(void) 추가 : 현재 running thread가 yield해야 하는지 검사를 위해 추가
![image](https://github.com/user-attachments/assets/7b7d565c-7cab-4d95-b8f7-c0bc7ecdf37d)


이 함수는, ready_list에서 가장 높은 우선순위의 스레드를 확인하는 함수로, Max_ready_t의 우선순위가 만약, 현재 실행 스레드의 우선순위가 크다면, 현재 실행되는 thread를 thread_yield를 통해 선점하도록 구현하였다. 

void thread_unblock (struct thread *); 함수 수정 : 우선순위에 따라 ready_list에 추가할 수 있도록 수정

![image](https://github.com/user-attachments/assets/86ba9f62-b48e-499b-b062-a9ede0bc0ae0)

원래 기존의 라운드로빈 방식에서는 주석처리된 코드와 같이, unblock된 리스트를 ready_list 뒤에 삽입하는 방식이였으나, list_insert_ordered()와 cmp_priority()를 사용하여 스레드를 ready_list에 자신의 우선순위에 따라 알맞은 위치에 삽입될 수 있도록 수정했다. 





void sema_down(struct semaphore *sema) 수정 : wait list에 추가될때도 우선순위에 정렬되도록  기존의 list_pushback을 사용하던 것을 list_insert_ordered()와 cmp_priority()를 사용하는 것으로 수정했다. 
![image](https://github.com/user-attachments/assets/1a603f70-9a9d-467e-8d7f-7198c93f1f3b)

void sema_up(struct semaphore *sema) 수정 : wait list에 가장 높은 우선순위의 스레드가 unblock되도록
![image](https://github.com/user-attachments/assets/bad0cbd3-3621-475f-b1a5-f6e62533dd90)


우선순위에 따라 정렬되어 있을 것이므로, 리스트 맨 앞 요소를 언블록하도록 했고, 추가적으로, test_max_priority(); 함수를 호출하여, 만약, 이렇게 unblock되었을 때에도, 해당 unblcok된 스레드가 현재 실행중인 리스트 보다 더 클 수 있으므로, 선점을 진행해야하는지 체크하고 필요하다면 진행하기 위해,  test_max_priority();를 호출하였다. thread_aging(void) 함수 추가 

![image](https://github.com/user-attachments/assets/12becd22-168e-4d68-84d8-75af846e558c)

우선순위가 높은 스레드가 cpu를 독점하는 것을 막기 위해, aging 함수를 구현했는데, 이는, ready_list에 있는 모든 스레드의 우선순위를 최대값이 넘지 않는 선에서 증가할 수 있도록 하는 역할을 한다. 
이는 매틱마다 timer_interrupt 함수에서 호출되는
![image](https://github.com/user-attachments/assets/76de5d02-7030-458e-af8d-9f28d4e4327b)

 thread_tick 함수에서 호출되어, 만약 aging의 값이 true면, ready list의 스레드들의 우선순위가 매틱마다 증가될 수 있도록 했다. 
![image](https://github.com/user-attachments/assets/0c61541d-246b-4fbc-8121-0d3633582197)

또한, thread_start 내에서도, 현재 스레드보다, 새로 create된 스레드의 우선순위가 높으면, yield 함수를 호출해서 선점이 가능하도록 함수를 수정했다. 
![image](https://github.com/user-attachments/assets/d510e5ff-d39c-40ab-84dd-066181fa807f)

thread_yield내에서도, yield한 스레드가 ready list에 삽입될 때, 우선 순위 순으로 삽입될 수 있도록 list_push_back 함수를 사용하던 부분을, -> list_insert_ordered와 cmp_pirority 비교자 함수를 사용하는 것으로 변경하였다. 

![image](https://github.com/user-attachments/assets/c5745330-64ef-4d9f-8b03-2d669498be4e)

![image](https://github.com/user-attachments/assets/cbae2bcc-1fb8-4ed8-a91a-26ae6036ef4e)

이 thread_set_priority() 함수는 현재 실행중인, 스레드의 우선순위를 변경하는 함수인데, 테스트 케이스 코드들 내에서 해당 함수가 호출되고 있었다. 우선순위가 변경되면, 이 변경된 우선순위가 레디 리스트의 타 스레드의 우선순위보다 더 작을 수가 있다. 이러한 경우에 대비해서 test_max_priority() 함수를 호출하여 선점을 확인하고, 해당 함수에서,  만약 더 높은 우선순위의 스레드가 있다면 현재 스레드는 CPU를 양보하게 될 수 있도록 했다. 그리고, mlfq 모드에서는 우선순위가 공식에 따른 계산을 통해 진행되기 때문에, pirority mode일때만 해당 로직이 진행되도록 구현하였다. 

#### mlfq. 
fixed_point_t fixed_point_operation(fixed_point_op_t op, fixed_point_t A, int B, fixed_point_t C)  : 연산 타입에 따라 필요한 여러 고정소수점 연산을 수행할 수 있도록 구현한 함수이다. 이 함수는 연산 종류와 피연산자를 인자로 받아 결과를 반환하는데,  이 은 fixed_point_operation() 함수를 호출해서  연산 매크로들구현하여, 실제 연산에 사용하였다. 

![image](https://github.com/user-attachments/assets/135cab6e-ac67-44ef-96a0-ae90d07e29e5)


void calculate_priority(struct thread *t, void *aux UNUSED) : thread의 우선순위를 mlfqs의 우선순위 계산 규칙에따라 계산할 수 있도록 구현한 함수로, 


이 우선순위 공식을 위에서 작성한 고정 소수점 연산 매크로를 이용해 구현하였다. 그리고, 우선순위가 범위를 넘으면 안되기 떄문에, 우선순위가 PRI_MIN과 PRI_MAX 사이에 있도록 검사도 진행하였다. 


void calculate_recent_cpu(struct thread *t, void *aux UNUSED) : recent cpu 값을 업데이트할 수 있도록 추가



이 공식이 적용되어 recent_cpu의 값이 계산될 수 있도록 했다. 

void calculate_load_avg(void): load_avg 값을  재계산)하도록 추가

이 공식을 통해, load_avg의 값이 계산될 수 있도록 했다. 

그리고 load_avg를 계산하기 위해선 레디 스레드 리스트의 크기도 필요하므로(현재 실행 스레드는 이들이 아니면 포함), 이를 위해 list_size 리스트 내장 함수를 이용하였다. 
thread_tick 함수 thread_tick() 함수에서 매 틱마다 mlfq의 우선순위 계산에 필요한 업데이트를 수행할 수 있도록 하였다. 스케줄링 모드가 mlfq일떄에만 이러한 업데이트가 필요한 것이므로, current schedulig모드로 스케줄링 모드가 어떤것에 해당하는지 switch - case문으로 조사할 수 있도록 하였다. mlfq 모드일 때,  매 틱마다 업데이트가 필요한 update_recent_cpu(t); 함수를 호출하였고, 매 1초마다 업데이트가 필요한 load_avg값과, recent_cpu 값은,  (ticks % TIMER_FREQ == 0) 이 조건을 만족시켰을 때에만 업데이트가 될 수 있도록 했다. 그리고,  매 타임 슬라이스마다 재계산 되어야 하는 우선순위는 (ticks % TIME_SLICE == 0 이 조건을 만족시켰을 때에만,   update_priority_and_yield(); 이 함수가 호출될 수 있도록 했다. 




static void update_recent_cpu(struct thread *t)-> 매 틱마다 실행 중인 스레드의 recent_cpu를 업데이트(증가)시킬 수 있도록 추가 

static void update_load_avg_and_recent_cpu(void) ->  매초마다, 모든 스레드의 recent_cpu와 load_avg를 재계산할 수 있도록 호출하기 위해 추가 thread_foreach를 사용해서, 실행 중인 스레드가 아닌, 모든 thread의 recent_cpu와 load_avg를 업데이트할 수 있도록 했다. 

static void update_priority_and_yield(void) -> 매 time_slice 매크로에 해당하는 틱마다, 모든 스레드의 우선순위를 재계산하도록 추가했다. 



void thread_set_scheduling_mode(bool mlfqs) 함수  추가 : 스케줄링 모드가 우선순위 모드 한가지만 있는 것이 아니므로, 각각의 모드대로 알맞은 동작을 수행할 수 있도록 해당 함수 추가 

현재 스케줄링 모드를 나타내는 열거형 scheduling_mode_t를 추가했고, 디폴트 값은 priority mode로 설정하였다. thread_set_scheduling_mode(bool mlfqs) 함수에서, init.c에서 mlfqs 커널 옵션에 따라 스케줄링 모드를 설정할 수 있도록 이 함수를 호출했다. 

이후에, thread 관련 코드에서 어떤 모드인지 확인하기 위해선 current_scheudling_mode의 enum 값이 무엇인지 확인하여 구분했다. 









init_thread 함수

thread_init 함수에서, 스케줄링 모드에 따라 사용되는 적절한 값이 초기화될 수 있도록 했다. 
그리고mlfq 모드에서  만약, 초기 스레드라면, nice값은 0, recent_cpu의 값은 0이 되도록 초기화 했고, 만약, initial thread가 아닌 경우라면, 현재 해당 스레드의 구조체에 담긴 기존의 nice, recent_cpu의 값을 활용할 수 있도록 했다.
그리고 이러한 값을 기반으로, priortiy를 calculate 할 수 있도록 호출했다. 추가적으로, load_avg 값의 초기화는 thread_start에서 0으로 진행하였다. 

thread_set_nice, thread_get_nice

먼저, set nice 함수를 호출하면,, 해당 thread에 인자로 전달된 nice value를 할당하고, nice 값이 우선순위 계산 공식에 변수로 포함되므로, 이에 따라 calculate_priority를 호출해서 현재 실행중인 스레드의 우선순위를 재계산하도록 했다. 또한, ready_list를,  재정렬 하고, 만약, 이렇게 재계산한 값에 따라, 선점이 진행되어야 할 수 있으므로, test_max_prirotiy를 호출하였다. 그리고 thread_get_nice 함수는 이를 호출하면, 해당 스레드의 nice 값을 조회할 수 있도록 수정했다. 

### 추가적으로 사용한 핀토스 내장 함수들 
sleep list ready list를 관리할 수 있도록 List_entry,list_insert_ordered(&sleep_list, &cur->elem, cmp_wake_up_time, NULL);,   list_sort(&ready_list, cmp_priority, NULL);등 다양한 리스트 내장 핀토스 함수들을 사용하였다. 



## 개발 중 발생한 문제나 이슈가 있으면 이를 간략히 설명하고 해결한 방식에 대해 설명

### 이슈 : 우선순위 기반 스케줄링을 Pintos에 구현하는 과정에서 priority-sema 테스트 케이스가 초기에는 실패한 문제
처음에는 Synch.c. 파일 내의 sema up/down 함수를 수정해야 한다는 것을 생각하지 못하였다. 하지만, priority-sema 테스트 케이스 코드를 보니, sema up과 sema down을 호출하고 있는 것을 확인하였다. 이를 통해, 분석을 해본 결과, 이 테스트 케이스는 여러 스레드가 세마포어를 기다릴 때, 우선순위가 높은 스레드가 먼저 깨어나고 실행되도록 하는지 확인하기 위해 설계된 것을 확인할 수 있었다. 하지만, 수정 전의  세마포어는 우선순위에 따라 세마포어의 wait가 관리되는 것이 아닌, 먼저 대기한 스레드가 먼저 깨어나는 방식으로 구현되어 있었다. 즉, 수정 전의 코드를 보면,  원래의 sema_down 함수는 list_push_back을 사용하여 대기 스레드를 리스트의 뒤에 추가했으므로, 우선순위가 높은 스레드가 먼저 깨어날 수 있는 구조가 아니었다. 이에 따라, 낮은 우선순위의 스레드가 먼저 깨어나게 되어 priority-sema 테스트 케이스가 실패하게된 것이였다. 따라서, 이 문제를 해결하기 위해, 세마포어의 대기자 리스트를 스레드의 우선순위에 따라 정렬되도록 수정하였다. 구체적으로, sema_down 함수에서 list_push_back 대신 list_insert_ordered를 사용하여 스레드를 우선순위 순으로 삽입하고, sema_up 함수에서도 우선순위에 따라 스레드를 언블록하도록 변경하였다. 또한, 이 케이스에서는 선점이 발생할 수도 있는 케이스이므로, 스레드가 언블록될 때 현재 스레드보다 우선순위가 높은 스레드가 있는지 검사하여 필요한 경우 CPU를 yield 할 수 있도록,  test_max_priority 함수를 호출하였다. 이를 통해 해당 테스트 케이스도 통과될 수 있게 되었다. 
#### <before>


#### <after>




## 시험 및 평가 내용
### priority-lifo.c 코드 및 priority-lifo 테스트 결과 분석
priority-lifo.c는 스레드 스케줄링의 우선순위 기반 LIFO(Last-In-First-Out) 정책을 테스트하기 위한 코드이다. 코드를 살펴보면,  즉, 스레드 ID가 생성된 시점을 나타내며, 가장 나중에 생성된 스레드가 가장 높은 우선순위를 가지도록 설정됨을 알 수 있다. 

이런식으로 생성된 순서에 따라 스레드에 우선순위를 부여하기 때문에, lifo 정책을 검증할 수 있는 것이다. 아래의 테스트 결과를 보면, 스레드가 생성된 순서와 반대로 실행됨을 확인할 수 있다. 즉, lifo 정책이 잘 동작함을 확인할 수 있다. 



### make check 수행 결과를 캡처하여 첨부


