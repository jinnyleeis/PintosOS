

# Pintos Project 2: User Program (2)
 
담당 교수 / 분반 :김영재
이름 / 학번 :이지인/20190094
개발 기간 :10/28~11/2


## 개발 목표
### 해당 프로젝트에서 구현할 내용을 간략히 서술
이번 프로젝트에선 파일 시스템과 관련한 시스템 콜들을 구현하는 것이 목표이다. 또한, 파일 디스크립터 테이블의 구현이 필요하며, 파일 시스템 처리 과정에서 발생할 수 있는 동기화 문제에 대해서도 적절한 동기화 메카니즘을 구현하는 것이 목표이다. 

## 개발 범위 및 내용
### 개발 범위
아래 각 항목을 구현해야 하는 이유, 혹은 구현 시 기대되는 결과를 간략히 서술
#### 1. File Descriptor
각 프로세스마다 파일과 관련된 작업을 하기 위해 파일 디스크립터 테이블을 구현해야한다. fd가 프로세스가 파일과 관련된 작업을 위해 필수적인 이유는, fd를 통해, 프로세스가 열린 파일을 추적하고 관리하기 때문이다. fd를 구현함으로써, 각각의 프로세스가 해당 파일에 대한 정확한 참조를 유지할 수 있게 되고, 각각의 프로세스별로 독립적으로 파일을 열고 닫고, 파일 작업을 수행할 수 있게 된다. 

#### 2. (이번 프로젝트에서 구현해야 하는) System Calls
이번에 구현해야하는 시스템콜은, create, remove ,open,close, filesize, read, write, seek, tell로, 이를 구현함으로써,  파일과 관련된 다양한 작업을 수행할 수 있게 된다. 이러한 시스템콜을 구현함으로써, 프로세스의 유저 프로그램이 파일과 관련된 시스템콜을 요청하면,  프로세스는 파일 생성, 삭제, 열기, 닫기, 읽기, 쓰기, 위치 이동, 현재 위치 확인 등의 작업을 효과적으로 수행할 수 있게 된다. 그리고, 프로젝트1에선, read, write 시스템콜에서, stdin과 stdout의 입출력 작업만 가능했는데, fd가 0,1인 경우뿐만 아니라, 더 확장하여, 일반적인 파일들에 대한 입출력 작업도 수행할 수 있게 된다. 

#### 3. Synchronization in Filesystem
파일 시스템은 여러 프로세스나 스레드가 동시에 접근할 수 있기 때문에 동기화 문제가 발생할 수 있다. 즉, 여러 프로세스가, 동시에 같은 파일을 열거나 수정하려고 하려고 할 수 있는데, 이런 파일 시스템은, 여러 프로세스가 동시에 접근할 경우 문제가 발생할 수 있는 critical section에 해당한다고 할 수 있는 것이다. 만약 이러한 Critical Section인 파일 시스템을 동기화 작업을 거치지 않으면, 데이터 손상, 일관성 문제, 시스템 충돌 등의 심각한 문제가 발생할 수 있다. 예를 들어, 두 개의 프로세스가 동시에 같은 파일에 데이터를 쓰려고 할 때, 동기화가 이루어지지 않으면 파일의 내용이 예기치 않게 섞이거나 일부 데이터가 손실될 수도 있는 것이다. . 이러한 문제를 방지하기 위해 파일 시스템의 Critical Section을 적절히 보호하는 동기화 메커니즘이 필요하다고 할 수 있다. 이러한 동기화 메커니즘을 구현해, 파일 시스템의 critical section을 보호할 수 있게 되면, 데이터 손상, 데이터 race 등의 문제를 해결할 수 있게되고, 멀티프로세싱 환경에서 안전한 파일 작업을 보장할 수 있게 되는 것이다. 

### 개발 내용
아래 항목의 내용만 서술
#### 1. File Descriptor: 구현에 이용할 자료구조와 선택한 이유를 서술
각 thread마다 배열의 자료구조를 통해, fdt를 구현하였다. 배열 자료구조를 선택한 이유는, Int형에 해당하는 파일 디스크립터 번호를 인덱스로 활용하여, 파일들을 관리하기에, 적합하기 때문이다. 또한, next_fd라는 다음에 할당될 파일 디스크립터 번호를 알려줄 수 있는 int형도 추가하였다. 왜냐하면, 파일 디스크립터 테이블은 맨 앞에서부터 비어있는 부분에 차례대로, 파일들을 할당하는데, 어디가 비어있는 가장 먼저 사용 가능한 파일 디스크립터 번호인지 추적할 필요가 있기 때문이다. 

#### 2. System Calls: 구현할 각 system call에 대해 간략히 서술 (하나의 system call 당 최대 3문장으로 간략히 설명; 3문장을 넘길 정도로 길게 작성하지 말 것)
write: 파일이나 콘솔에 데이터를 쓸 수 있게 한다. 파일 디스크립터와 버퍼, 크기를 인자로 받아 데이터를 출력하는 역할을 한다. 이번 프로젝트에선 기존에 구현하였던 콘솔 이외에 파일에도 데이터를 쓸 수 있는 기능을 추가구현했다. 
read: 파일이나 콘솔로부터 데이터를 읽는다. 파일 디스크립터와 버퍼, 크기를 인자로 받아 데이터를 입력받는다.  이번 프로젝트에선 기존에 구현하였던 콘솔 이외에 파일의 데이터도 읽을 수 있는 기능을 추가 구현했다. 
create: 새로운 파일을 생성한다. 이 함수는 파일 이름과 초기 크기를 인자로 받아 파일 시스템에 새로운 파일을 만든다. 
remove: 지정한 파일을 삭제한다. 파일 이름을 인자로 주어, 파일 시스템에서 해당 파일을 제거한다.
open: 파일을 열고, 파일 디스크립터를 반환한다. 파일 이름을 인자로 받아 파일을 열고, 해당 파일을 관리할 수 있는 파일 디스크립터를 반환한다.
close: 열려 있는 파일을 닫는다. 파일 디스크립터를 인자로 받아 해당 파일을 닫고, 파일 디스크립터 테이블에서 제거한다.
filesize: 열린 파일의 크기를 반환한다. 파일 디스크립터를 인자로 받아 해당 파일의 크기를 확인할 수 있게 한다. 
seek: 열린 파일의 현재 위치를 변경한다. 파일 디스크립터와 새로운 위치를 인자로 받아 파일 포인터를 이동시켜준다고 할 수 있다.
tell: 열린 파일의 현재 위치를 반환한다. 파일 디스크립터를 인자로 받아 파일 포인터의 위치를 확인한다고 할 수 있다. 

#### 3. Synchronization in Filesystem: Lock, Semaphore를 어떻게 이용할 수 있는지 각각에 대해 설명 (다른 방법을 서술해도 되지만 lock과 semaphore는 반드시 포함해야 함)
lock과 semaphore를 이용한 경우를 각각 나누어 설명하겠다. 
락을 사용하여, 파일 시스템에 접근하는 모든 시스템 콜에서 전역적인 락으로 critical section을 보호할 수 있게 한다. 즉, 파일 시스템과 관련된 시스템콜에서 파일에 접근하기 전에, 락을 획득하고, 파일 작업이 끝아면, 락을 해제하여, 여러 프로세스가 동시에 파일 시스템에 접근할 때 발생할 수 있는 데이터 손상이나 일관성 문제 등의 동기화적 문제를 방지한다. 
세마포어를 사용한 부분은 프로젝트1에서 구현한 부분인데, 프로세스의 로드 및 종료 상태를 부모 프로세스와 동기화하여, 프로세스 간의 의존성을 관리하는데 세마포어를 사용한다. 즉, 프로세스의 종료 시 자원을 적절히 해제하고, 부모 프로세스가 자식의 종료를 기다림으로써 리소스의 leak이 일어나는 것을  방지할 수 있게 된다. 




## 추진 일정 및 개발 방법
### 추진 일정
#### II. A. 개발 범위를 포함하여 구현 내용에 대한 일정 작성
10/28-30 파일 디스크립터와, 구현해야 하는 시스템 콜들의 기능을 1차적으로 구현하여, syscall handler에서 호출할 수 있도록 구현하였다. 
10/30 그 후 락을 사용하여, 적절히 동기화가 될 수 있도록 하였다. 
10/31 그 후로, 동기화가 제대로 잘 안되고 있는, write 관련 부분(deny하면 안되는 파일들에 대해서도 deny되는 문제) 등을 손보았고, 
11/1-2에는 중복되는 공통 로직을 함수로 뺴내어 리팩토링을 진행하고, implicit declaration 경고를 없애었다. 

![image](https://github.com/user-attachments/assets/ef1a1699-df4a-4c10-91dd-835ca2d143f0)



### 개발 방법
#### II. B.의 개발 내용을 구현하기 위해 각각에 대해 다음 사항들을 포함하여 설명
##### 수정해야하는 소스코드
1. file descriptor 
threads/thread.h: 스레드 구조체에 파일 디스크립터 테이블과 관련 변수 추가
threads/thread.c: 파일 디스크립터 테이블 초기화 및 관리 함수 구현

2. system call
userprog/syscall.c: 시스템 콜 핸들러 및 각 시스템 콜 함수 구현
synchronization 
userprog/syscall.c : 파일 시스템을 접근하는 모든 함수에서 락 획득 및 해제 코드 추가
userporg/process.c : 프로세스의 로드 및 종료 시 fd 테이블과 실행 파일에 대한 관리를 추가 -> 리소스를 적절히 관리

##### 수정하거나 추가해야 하는 자료구조
1) file descriptor 
핀토스 공식문서 35p에 언급된 것과 같이, 각 프로세스당, open file에 임의로 제한을 그때마다 설정할 수 있게 하는 것보다, 고정적으로 max를 128개로 설정하는 것이 좋다고 하여 이에 따라, 128의 크기를 가진 file descriptor table 배열 자료구조를 struct thread내에 추가하였다. index가 file descriptor에 해당하고, 배열에 저장되는 value는, struct file에 해당한다. 
reserved_entry라는 배열을 사용하여 이미 정해진 예약된 파일 디스크립터 항목을 지정한다. 따라서, 파일 디스크립터의 엔트리가 이 항목 이후로, 사용될 수 있도록 한다. (그냥 숫자 2로 사용하는 것보다, 현재 핀토스 프로젝트에는 stderr에 해당하는 fd 2가 없는 것으로 안다. 하지만, 추후에 이런 reserved할 엔트리를 추가할 수 있으므로,reserved_entry 자료구조를 별도로 추가한다. )

2) 전역 락 filesys_lock : 락을 특정 프로세스가 획득할 수 있도록 Syscall.c에 추가해야함

#####수정하거나 추가해야 하는 함수
init_thread(): 스레드 생성 시 파일 디스크립터 테이블과 관련 변수 초기화필요 
파일 디스크립터의 유효성을 검사하기 위한 함수 validate_fd와,open, close 등의 여러 시스템콜들
is_opening_file, is_closing_file 함수를 구현하여, 특정 프로세스에서 파일을 닫거나 열려는 행위가 동기화 측면에서 합당한 행위인지 검사할 수 있도록 함 -> 유효하지 않으면, 종료, 유효하면, file struct를 반환하여 이용
syscall_handler(struct intr_frame *f): 프로젝트2의 시스템 콜 번호를 확인하고, 해당하는 시스템 콜 함수를 호출할 수 있도록 함 
동기화 관련하여서 파일 시스템을 접근하는 모든 함수에서 락 획득 및 해제 코드 추가 필요 
process.c에서도 프로레스의 로드 및 종료시에, fdt와 실행 파일에 대한 관리가 가능하게 load() 함수와 process_exit() 내 함수 수정이 필요함



## 연구 결과
### Flow Chart
II. B. 개발 내용의 각 3가지 항목에 대하여 Flow Chart 작성
#### file descriptor , synchronization
![image](https://github.com/user-attachments/assets/ba4fa4a8-b51b-4aef-9e88-e78a150bddf9)





#### system call (write를 예시로 들어 설명하였습니다.)
![image](https://github.com/user-attachments/assets/14e2ae48-897d-49de-9bc3-1fb4fff34549)






### 제작 내용
II. B. 개발 내용의 각 3가지 항목에 대하여 실직적으로 구현한 코드의 관점에서 작성 (구현 내용, 알고리즘 등을 명확히 서술할 것)
1.thread.h
![image](https://github.com/user-attachments/assets/c12c87b9-c218-4b5e-9071-67cbbe65d328)
![image](https://github.com/user-attachments/assets/6af6de2b-a221-4291-8fbe-aef438feb41e)

스레드 구조체에 파일 디스크립터 테이블과 관련 변수를 추가하여 각 스레드가 관리하는 파일들을 추적할 수 있도록 하였다. 우선, 파일 디스크립터 테이블의 크기를 정의하는 매크로를 추가하였다. 또한, 파일 디스크립터 테이블을 초기화할 때 예약된 항목(standard input과 output, 즉 0과 1)을 설정할 수 있도록, int reserved_entry[2]에 해당하는 배열을 선언하였다. 또한, struct thread내에서 ftd 배열을 생성할 때, 매크로를 사용하여 배열의 크기를 정의하였다. 그리고,  다음에 할당할 파일 디스크립터 번호를 나타낼 next_fd 변수도 추가하였다. 그리고,  현재 실행 중인 프로세스의 실행 파일을 참조하여,  실행 중인 프로그램의 안정성을 보장할 수 있도록, 실행 파일에 대한 파일 포인터인 exec_file도 멤버로 추가하였다. 

2. thread.c
![image](https://github.com/user-attachments/assets/0efba6cd-d290-4f7b-a9a6-b99be49609ee)

init_thread 함수 내에서 t->reserved_entry를 사용하여 예약된 항목 배열을 참조해, 이를 먼저 초기화할 수 있도록 하였다. 이를 통해, 0,1을 Stdin stdout을 위해 사용할 수 있도록 했다. 
이후, next_fd 초기화를 위해 reserved_entry 배열의 크기를 계산하여, 예약된 항목의 개수로 초기화할 수 있도록 했다. 이를 통해, 예약되지 않은 맨 앞의 항목부터 next_fd가 가리킬 수 있도록 하였다. 다음으로,  init_thread 함수에서 파일 디스크립터 테이블을 초기화할 때 매크로와 예약된 항목을 사용하여 초기화를 진행할 수 있도록 했다. 아직, 파일 디스크립터가 특정 file struct를 가리키고 있는 것은 없으므로, 다 null로 초기화하였다. 아직, 실행 파일도 없는 상태이므로, 실행 파일에 대한 파일 포인터도 null도 초기화하였다. 

3. process.c
프로세스의 로드 및 종료 시 파일 디스크립터 테이블과 실행 파일에 대한 관리하는 부분을 추가하였다. 
![image](https://github.com/user-attachments/assets/77db0a22-233c-49bd-b852-edd900c0f959)

먼저 load() 함수 내에서 만약,  프로세스가 성공적으로 로드되었다면, 성공적으로 로드된 실행 파일에 대한 참조를 스레드 구조체에 저장할 수 있도록 하였다. 이를 통해, 이후에  실행 중인 프로그램의 실행 파일을 보호하기 위해 사용할 수 있도록 하였다. 

![image](https://github.com/user-attachments/assets/a93263a1-d8df-4161-b7b6-fd57a303533d)


다음으로, process_exit에서는, 프로세스가 종료될 때, 실행 파일에 대한 쓰기를 허용하고 파일을 닫을 수 있도록 하였다. 이러한 작업이 필요한 이유는, process_exit는 실행 중인 프로세스가 종료될 때 호출되는 것이므로, 프로세스가 종료되면, 프로세스에서 실행중이던 파일에 대한 lock 권한도 해제하여, 다른 프로세스가 실행 파일에 대한 쓰기를 할 수 있도록 허용하고, 지금의 프로세스에선 해당 실행 파일을 닫는 작업이 필요할 것이다. 이를 위해, 해당 프로세스의 실행 파일이 Null이 아닐 경우에, 파일을 닫고, 실행파일을 null로 만드는 작업을 수행했다. 
실행 파일 뿐만 아니라, 해당 프로세스의 파일 디스크립터가 가리키고 있는 파일 구조체들이 있다면, 이또한 연결을 끊어야 한다. 왜냐하면, 프로세스가 종료됬는데 이러한 연결을 끊지 않으면, 종료된 프로세스의 포인터가 특정 파일 구조체를 가리키는 것은 사용되지 않을 것이기 때문에, 자원의 낭비이므로, null로 모든 연결을 끊고, 열린 파일을 닫았다. 


4. syscall.c 
파일 시스템에 접근하는 모든 시스템 콜에서 전역 락을 사용하여 동기화 문제를 해결할 수 있도록 했다. 
![image](https://github.com/user-attachments/assets/e0e0f836-e6eb-4f2d-8841-0c180eacf60c)

![image](https://github.com/user-attachments/assets/7f8e7518-4905-4021-bea4-59f45ad8662d)

시스템 부팅시, 전역락이 초기화될 수 있도록 syscall init내에 lock init으로, 전역 락을 초기화 했다. 

close() 
![image](https://github.com/user-attachments/assets/d1cdf72b-8456-43a8-9dd1-984bcf93efc5)

validate_fd 함수를 통해, 해당 fd가 유효한 fd인지 검사했다. 만약, 유효한 fd면, 해당 fd가 가리키는 file struct 포인터를 반환한다.  또한, threa_current를 통해, 현재 thread를 가리키는 포인터를 받아온다 fd 유효성 검사를 했으므로, 파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득하여 close 작업을 수행할 수 있도록한다. 
그 후, is_closing_executable_file(file) 함수를 호출하여 닫으려는 파일이 실행 파일인지 확인하고, 만약, 실행 파일인 경우에는 true를 리턴받는다. true인 경우에는, 현재 프로세스의 실행 파일에 대해, 모든 프로세스들이 wrtie하는 것을 allow하고, 현재 프로세스에선 exec_file 포인터를 NULL로 설정한다. 아니면,  is_closing_executable_file(file) 함수가 false를 반환, 즉, 일반 파일인 경우에는, 실행 파일과 달리 실행 중인 프로세스가 파일의 쓰기를 제한하는, file_deny_write를 호출하지 않았을 것이므로, , file_allow_write를 호출할 필요 없이, 단순히, file_close(file)를 호출하여 파일을 닫고, 파일 디스크립터 테이블에서 해당 fd를 NULL로 설정한다. 그후, 파일 시스템 락을 해제하여 다른 프로세스가 파일 시스템에 접근할 수 있도록 한다. 

open()
![image](https://github.com/user-attachments/assets/f328abe9-dba0-4105-bce5-2f5ad1cb7159)

먼저, check_valid_string(file) 함수를 호출하여 유효성을 확인한 후, filesys_lock 획한다.. 그 후, filesys_open(file) 함수를 호출하여 파일을 열고, 만약, 파일이 존재하지 않거나 열 수 없는 경우 NULL을 반환할 것이므로, 이때에는, 파일 열기에 실패한 경우일 것이므로, 파일 시스템 락을 해제하고 -1을 반환한다. 만약, 파일이 성공적으로 열렸다면, 현재 프로세스를 struct thread 구조체를 받아온다. 그 후, ​​is_opening_executable_file(file) 함수를 호출하여 열려는 파일이 현재 실행 중인 프로세스의 실행 파일인지 확인한다. 만약, 현재, 실행 파일인 경우에는, 프로세스가 자신의 실행 파일을 실행 중일 때, 해당 파일을 수정하지 못하도록 보장할 필요가 있으므로,  현재 프로세스의 실행 파일에 대해, 파일 자체에 대한 쓰기 접근을 금지한다. 즉, file_deny_write는 파일에 대한 모든 쓰기를 금지하므로, 실행 파일을 열고 있는 프로세스 자신도 해당 파일에 쓰기를 시도할 수 없을 뿐만 아니라, 다른 프로세스들의 쓰기를 file_deny_write(opened_file) 호출을 통해 금지하는 것이다.  하지만, 그냥 열기만한 일반 파일인 경우에는, 이런 쓰기 금지 함수를 호출하지 않는다. 그 후, next_fd를 받아와, 해당 파일 디스크립터가 이 열린 파일을 가리키도록 하고, 다음에 할당될 파일 디스크립터 번호를 증가시킨다. open과 관련된 작업이 다 끝났으므로, 파일 시스템 락을 해제하고, fd를 리턴한다. 



read()
![image](https://github.com/user-attachments/assets/bc4a150c-9d19-4be1-87bb-3a2dd7337025)

validate_fd 함수를 호출하여 fd가 유효한 파일 디스크립터인지 확인하고, 유효한 경우 파일 포인터를 반환하고, 유효하지 않은 경우 프로세스를 종료한다. 파일 디스크립터 fd가 표준 입력(콘솔 입력)인지 확인한 후에는, 프로세스간 파일 시스템 criticla section과 관련한 동기화 작업 없이, 그저, 콘솔에서 읽기 작업을 진행하고, 읽은 바이트 수를 계산해 반환하면 된다. 하지만, fd가 0이 아닌 경우에는,  파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득한다. 그후,  file_read(file, buffer, size) 함수를 호출하여 파일에서 데이터를 읽고, 파일 시스템 락을 해제하고, 읽은 바이트 수를 사용자 프로그램에 반환한다. 











write()
![image](https://github.com/user-attachments/assets/f5c9ba5b-1dc7-4932-ac28-4e97ef8fd6a8)

이 경우에도, fd가 1인 경우는 락과 관련된 작업 없이, putbuf를 통해, 콘솔에 버퍼의 데이터를 출력한다. 그 후, length를 리턴한다. fd가 stdout이 아닌 경우에는, 파일에 데이터를 쓰는 경우로, if문을 통해, fd가 유효한 파일 디스크립터인지 확인한다. 유효한 경우, 파일 시스템의 동시 접근을 방지하기 위해 락을 획득한다  파일 포인터 cur->fdt[fd]를 인자로 주어, file_write(file, buffer, size) 함수를 호출하여 파일에 데이터를 쓰는 작업을 수행하고, bytes_written 변수에 실제로 작성된 바이트 수를 저장한다. 이후, 파일 시스템 락을 해제하여 다른 프로세스가 파일 시스템에 접근할 수 있도록 하고, , bytes_written를 반환한다. 









filesize()
![image](https://github.com/user-attachments/assets/76a0c04b-6c4c-400f-a16b-81372c955e25)

사용자 프로그램에서 filesize 시스템 콜이 호출되면, 전달된 파일 디스크립터 fd의 유효성을 validate_fd 함수를 통해 검사한다. 파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득한 후, file_length 함수를 호출하여 파일의 크기를 가져온다. 가져온 파일 크기를 size 변수에 저장하고, 락을 해제한다. 최종적으로 파일의 크기를 사용자 프로그램에 반환한다.

seek(), tell()
![image](https://github.com/user-attachments/assets/854fbcd3-8170-4a9d-83a5-55b1af64e0f9)

사용자 프로그램에서 seek 시스템 콜이 호출되면, 전달된 파일 디스크립터 fd의  유효성을 validate_fd 함수를 통해 검사한다. 파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득한 후, file_seek 함수를 호출하여 파일의 현재 위치를 지정된 position으로 변경한다. 위치 변경이 완료되면, 락을 해제한다. 최종적으로 프로세스는 정상적으로 종료된다.
사용자 프로그램에서 tell 시스템 콜이 호출되면, 전달된 파일 디스크립터 fd의 유효성을 validate_fd 함수를 통해 검사한다. 파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득한 후, file_tell 함수를 호출하여 파일의 현재 위치를 가져온다. 가져온 파일 위치를 position 변수에 저장하고, 락을 해제한다. 최종적으로 현재 파일 위치를 사용자 프로그램에 반환한다.

create(), remove()
![image](https://github.com/user-attachments/assets/fa74fc5b-f9a0-4015-a8ed-f4e8d22d1825)

사용자 프로그램에서 create 시스템 콜이 호출되면, 전달된 파일 이름의 유효성을 check_valid_string 함수를 통해 검사한다. 파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득한 후, filesys_create 함수를 호출하여 파일을 생성한다. 파일 생성 성공 여부를 success 변수에 저장하고, 락을 해제한다. 최종적으로 파일 생성에 성공하면 true를, 실패하면 false를 반환한다.
사용자 프로그램에서 remove 시스템 콜이 호출되면, 전달된 파일 이름의 유효성을 check_valid_string 함수를 통해 검사한다. 파일 시스템의 동시 접근을 방지하기 위해 filesys_lock을 획득한 후, filesys_remove 함수를 호출하여 파일을 삭제한다. 파일 삭제 성공 여부를 success 변수에 저장하고, 락을 해제한다. 최종적으로 파일 삭제에 성공하면 true를, 실패하면 false를 반환한다.

#### 구현에 있어 Pintos에 내장된 라이브러리나 자체 제작한 함수를 사용한 경우 이에 대해서도 설명
![image](https://github.com/user-attachments/assets/19e2f847-5ded-4de9-98da-ca3fbe4675d5)

1. is_closing_executable_file 
파일을 닫을 때, 해당 파일이 현재 프로세스의 실행 파일인지 확인하는 함수이다. 현재 실행 중인 스레드(프로세스)를  thread_current()를 통해 가져온 후, 전달된 파일 포인터(file)가 현재 스레드의 실행 파일 포인터(cur->exec_file)와 동일한지 비교하고, 이 비교의 불리언 값을 리턴한다. 
2.  is_opening_executable_file 
파일을 열 때, 해당 파일이 현재 프로세스의 실행 파일인지 확인하는 함수이다. 사실, 위의 closing일때와 동일한 기능을 수행하는 함수이나, 이때는, 핀토스가 C 표준 라이브러리의 일부 함수들을 자체적으로 구현한 것인, 문자열 비교 함수 strcmp를 사용하였다. 
3. validate_fd 함수 
![image](https://github.com/user-attachments/assets/4ca1e93a-78f4-4c13-8e42-825dfa655073)

사용자 프로그램이 전달한 파일 디스크립터(fd)가 유효한지 확인하고, 유효한 경우 해당 파일 포인터를 반환하고, 반대로 유효하지 않은 경우 프로세스를 종료시켜주는 함수이다. 이를 통해, 잘못된 파일 디스크립터 접근 문제를 막는다. 현재 실행 중인 스레드(프로세스)를 가져오고, 만약, 파일 디스크립터가 0 또는 1인 경우는 표준 입력(0) 또는 표준 출력(1)으로, 파일 디스크립터로는 유효하지 않다고 판단하여 exit(-1)을 호출한다. 그리고, 파일 디스크립터가 2 이상이고, 현재 스레드의 next_fd 미만, 그리고,fdt에서 해당 fd가 NULL이 아닌 경우에는, 유효한 파일 디스크립터로 간주하여 해당 파일 포인터를 반환한다. 이 두 경우에 해당하지 않는 경우에도 유효하지 않다고 판단해, exit(-1)을 호출한다. 

4. 파일 관련 함수들 
시스템콜에 대한 설명에서 설명한 부분이나, 각각의 파일 관련 시스템 콜에 적합한 핀토스 내장 함수를 사용하였다. 
open에선, filesys_open, close에선 file_close, wrtie에선 file_write, filesize에선 file_length(file);, seek에선 file_seek(file, position);, tell에선  file_tell(file);를 사용하였다. 또한 lock을 관리하기 위해,   lock_acquire(&filesys_lock);,  lock_release(&filesys_lock);를 사용하였고, 실행 파일에 대한 쓰기 권한을 관리하기 위해,     file_allow_write(file);와,     file_deny_write(opened_file);도 사용하였다. 






### 개발 중 발생한 문제나 이슈가 있으면 이를 간략히 설명하고 해결한 방식에 대해 설명
#### 이슈1) 
![image](https://github.com/user-attachments/assets/ab3e54a6-84f0-4901-add7-7d022ff316b2)

write-normal의 테스트 케이스에 대해 run: write() returned 2 instead of 239: FAILED로 계속해서 fail이 나왔다. 이때, write 함수 내에서 printf 문을 통해, 디버깅을 하였는데, (아래에 주석 처리된 부분) bytes_written은 알맞게 239를 담고 있음을 확인하였다. 하지만, 테스트 케이스에서는(유저 프로그램에선) 2를 반환하는 것이였다. 이를 통해서,  write() 함수의 반환값을 사용자 프로그램에게 제대로 전달하지 않은 것이 문제임을 확인할 수 있었다. 

![image](https://github.com/user-attachments/assets/a9737f92-4a9b-454a-8bf7-fbc321a7af18)

위는, f->eax에 반환하지 않은 수정 전 문제의 코드이다. 이에 f->eax에 할당되게 했더니 유저 프로그램에도 제대로 된 값이 저달되면서 write-normal 테스크 케이스를 통과할 수 있었다. 

#### 이슈2) 
FAIL tests/userprog/rox-child run: write "child-rox": FAILED
FAIL tests/userprog/rox-multichild run: write "child-rox": FAILED

rox-simple 테스트 케이스는 성공했으나, 위의 두 테스트케이스가 계속해서 실패하였다. 
아래는 문제의 디버깅 결과이다. 부모(rox-child)에서 child-rox 파일에 file_write가 16을 반환하며 쓰기가 성공하는 것을 확인할 수 있다. 그리고 child-rox(자식)에선,  자신의 실행 파일인 child-rox에 대한 쓰기 접근을 거부하는 것을 볼 수 있다. 이후 child process가 종료된 후, 부모에서 다시, child-rox 파일에 write를 실행하면, write가 성공해야 한다. 하지만, 부모 프로세스의 write가 fail되는 것을 볼 수 있다. 원래 부모 프로세스가 쓰기 작업중이던 파일이고, 아직 close한 상태가 아니므로, 이 작업은 성공하는 것이 맞을 것이다. 
코드를 보니, load 함수 내에서, 파일을 연 후에, file_deny_write를 호출해, 자식의 실행 파일 child-rox 파일에 대한 쓰기 제한이 적용된 상태였다. 이로 인해, 자식 프로세스가 종료된 후에, 다시 부모 프로세스로 돌아올 때, 부모 프로세스(rox-child)가 자신이 open한 (child-rox)에 대해 write를 시도했으나, load내에서 file_deny_write가 설정되어 있고, 이게 해제되지 않은 상황이여서(자식 프로세스가 load 될 때, exec_file= child-rox 파일에 대해, file deny write가 설정되고 해제되지 않은 상황임), 부모 프로세스의 write가 실패한 것이라고 파악할 수 있었다. 
때문에, load내의  file_deny_write를 제거하고, 프로세스가 독립적으로 자신의 실행 파일을 열 때만 file_deny_write(opened_file);을 호출하여 쓰기 제한을 설정하는 것을 명확히 할 수 있도록, open() 함수에서 file_deny_write를 호출하는 것으로 변경하였다. 
또한, 파일을 닫을 때, close 함수에서 쓰기 제한 해제를 하고 실행 파일을 null로 만들 수 있도록 하여, 파일을 닫을 때, 쓰기 제한을 적절히 해제할 수 있도록 하였다. 
이러한 작업을 통해, 부모 프로세스(rox-child)는 child-rox 파일에 대해 쓰기가 허용되어 정상적으로 동작 -> 자식 프로세스(child-rox)는 자신의 실행 파일에 대해 쓰기가 제한되어 쓰기 실패 -> 다시 부모로 돌아왔을 때, child-rox 에 대한 파일 쓰기 허용을 달성할 수 있었다. 




##### before 
![image](https://github.com/user-attachments/assets/eba10a5b-ee3f-4ea5-9f4b-660952b2618e)

##### after
![image](https://github.com/user-attachments/assets/7489fdf5-7a0d-4ddd-8bf0-05b6236e8dc8)


## C. 시험 및 평가 내용
make check 수행 결과를 캡처하여 첨부
![image](https://github.com/user-attachments/assets/f99cb04f-3a92-4b4d-91a9-3528f84fb544)