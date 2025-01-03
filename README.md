# 운영체제 2024-2 수업에서 개발한 핀토스 프로젝트(개발 기간 9월-12월) 

[1. user program 1 - 테스트용2-추가 브랜치에서 개발](#-Pintos-Project-1:-User-Program-(1))
[2. user program 2 - 테스트용2-추가 브랜치에서 개발](#-Pintos-Project-2:-User-Program-(2))
3. thread - 테스트용2-추가 브랜치에서 개발
4. virtual memory - feature/exit-message 브랜치에서 개발

# Pintos Project 1: User Program (1)

담당 교수 : 김영재

조 / 조원 : 이지인

개발 기간 : 9/12~10/3

1. **개발 목표**
- **해당 프로젝트에서 구현할 내용을 간략히 서술.**

이번 유저 프로그램 과제의 개발 목표는 Pintos OS에서 사용자 프로그램의 실행과 시스템 콜을 구현하는 것이다. 이를 위해 먼저 인자 전달(Argument Passing)을 구현하고, 스택에 인자를 적재하여 시스템 콜 처리 과정에서 활용할 수 있도록 한다. Exit, Wait, Exec 등의 시스템 콜을 구현하여 유저 프로그램이 시스템 콜을 요청하면 커널이 해당 작업을 대신 수행할 수 있어야 한다.특히, 사용자 프로그램이 잘못된 포인터(NULL 포인터, 매핑되지 않은 가상 메모리, 커널 주소 공간의 포인터 등등)를 전달할 수 있으므로, 이러한 유효하지 않은 포인터를 안전하게 처리하여 커널이나 다른 실행 중인 프로세스에 해를 끼치지 않도록 해야 한다. 따라서 메모리 접근 시 접근 가능한 영역만 안전하게 접근할 수 있도록 해야 한다.

**개발 범위 및 내용**

1. **개발 범위**
- **아래 항목을 구현했을 때의 결과를 간략히 서술**
1. Argument Passing

argument passing 기능이 구현되지 않았을 때, 프로그램에서 명령어(예시:echo x)를 실행하려고 하면, 프로그램 이름과 인자 분리 작업이 제대로 이루어지지 않았다. 이로 인해 실행할 프로그램인 echo와 그 뒤의 인자들이 올바르게 처리되지 못하여 다음 사진과 같은 오류가 발생하였다.’

![image](https://github.com/user-attachments/assets/95bd427b-5071-446c-b194-1837f8a34a65)


argument passing 기능을 올바르게 구현한 후, 명령어(예시:echo x y)는 정상적으로 처리되었다. 프로그램 실행 시 프로그램 이름인 echo와 인자인 x y를 정확하게 분리하고, 이를 스택에 적절히 할당한 후 프로그램을 실행할 수 있게 되었다. 아래는 hex_dump()를 통해, 스택에 parsing한 argument가 적절히 쌓였는지 확인해본 터미널 출력 결과이다.

![image](https://github.com/user-attachments/assets/d71a22b2-4697-4e1c-92bf-a967d840904e)


argument passing이 성공적으로 작동하니, 프로그램은 올바르게 실행되어, 9번(write)에 해당하는 적절한 시스템 콜이 호출되었다.

2. User Memory Access

User Memory Access를 구현함으로써 커널 패닉과 페이지 폴트 문제를 해결할 수 있었다. User Memory Access 구현 이전에는 커널이 잘못된 사용자 메모리 접근으로 인해 kernel panic 또는 page fault 문제가 아래와 같이 발생하였다. syscall.c에서는 시스템 콜 인자로 전달되는 메모리 주소에 대한 유효성 검사가 없었고, 잘못된 주소를 그대로 참조하다가 커널 패닉이 발생했다. 또한, process.c에선  함수에서 스택에 인자를 넣는 과정에서 스택 포인터 유효성 확인이 누락되었기 때문에, 잘못된 스택 주소로 인해 메모리 오류가 자주 발생했다. 하지만, User Memory Access를 구현함으로써 커널 패닉과 페이지 폴트 문제를 해결할 수 있었다.

- 기존에 발생했던 오류에 관한 터미널 출력들
![image](https://github.com/user-attachments/assets/63f29ee9-7239-4513-b004-4b73372f7d45)
![image](https://github.com/user-attachments/assets/cf4c4abf-6f0e-4fcd-a32d-19121371b49e)


3. System Calls

시스템 콜이 구현되기 전에는 프로그램이 echo x y 명령어를 실행할 때 시스템 콜 번호가 올바르게 표시되었으나, 실제 출력 기능이 작동하지 않아 터미널에 출력되지 않았다. 시스템 콜을 호출하는 부분은 있었지만, 기능적으로 제대로 처리되지 않았기 때문에 출력이 이루어지지 않은 것이였다. 시스템 콜을 적절히 구현한 이후에는 echo x y 명령어를 실행할 때 시스템 콜을 통해 제대로 처리되었고, 터미널에 echo x y가 출력되었다. 시스템 콜이 프로그램 내부에서 제대로 호출되고, 이를 통해 표준 출력 장치로 데이터가 전달되면서 출력 기능이 정상적으로 동작하게 된 것이라 할 수 있다. write 시스템콜을 적절히 구현하였을 때, 위와 같은 결과를 확인할 수 있었고, 이외에 나머지 시스템콜을 구현하고 적절히 작동함은 여러 테스트 케이스에 대한 결과가 pass가 뜨는 것을 확인하여 알 수 있었다.

1. **개발 내용**
- **아래 항목의 내용만 서술 (기타 내용은 서술하지 않아도 됨.)**
- Argument Passing
    - 커널 내 스택에 argument를 쌓는 과정 설명

커널은 프로그램에 전달된 인자들을 스택에 쌓아 올려 유저 프로그램이 이를 접근할 수 있도록 준비한다. 이때, 인자들을 메모리의 상위 주소부터 역순으로 인자들을 메모리의 상위 주소부터 역순으로 쌓는다. 그후, 각 인자의 주소를 쌓는다. 그 다음으론,  인자의 주소 배열에 해당하는 argv와 인자의 개수 argc도 스택에 저장하여, 유저 프로그램이 해당 인자들을 쉽게 접근할 수 있도록 한다.

아래는 echo x y를 기준으로 유저 스택에 쌓은 예시이다. (아래는 hex_dump()로 디버깅하여 나온 결과에 해당한다.)

![image](https://github.com/user-attachments/assets/c147643a-e55a-413a-b7af-71865427377c)


최상단 (PYHBASE쪽에 해당)

---

1. y\0
2. x\0
3. echo\0
4. 패딩 (4의 배수로 맞추기 위함)
5. NULL sentinel
6. argv[2] (y)의 주소
7. argv[1] (x)의 주소
8. argv[0] (echo)의 주소
9. argv 주소
10. argc
11. fake return address

---

하단

---

- User Memory Access
    - Pintos 상에서의 invalid memory access 개념을 간략히 설명

Pintos에서 invalid memory access란 사용자 프로그램이 접근해서는 안 되는 메모리 영역, 즉 할당되지 않았거나 커널 영역에 있는 메모리에 접근하는 것을 의미한다.

- Invalid memory access를 어떻게 막을 것인지 설명

위와 같이 사용자 프로그램이 접근해서는 안되는 메모리 영역을 접근하지 못하도록 하기 위해서는,  메모리 접근 시 커널은 is_user_vaddr() 함수로 주소가 사용자 영역에 속하는지 확인하고, pagedir_get_page()로 해당 주소가 실제로 매핑된 페이지인지를 확인해 유효성을 검사해야 한다.

- System Calls
    - **시스템 콜의 필요성에 대한 간략한 설명**

시스템 콜이 필요한 이유는, 하드웨어에 접근해서 특정 작업을 수행하거나, 파일 시스템을 사용하는 것은, 유저 프로그램이 직접적을 수행할 수 없는 일이기 때문이다. 왜냐하면, 만약, 유저 프로그램에게 직접적으로 이러한 권한을 부여할 시에 ,시스템의 안정성과 보안을 해칠 수 있기 때문이다. 따라서, 운영체제는 시스템 콜을 통해 유저 프로그램이 커널 모드로 전환하여 필요한 작업을 요청하고, 유저 프로그램 대신, 커널이 해당 작업을 대신 수행해준다.

따라서, 유저 프로그램이 시스템콜 인터페이스를 통해, 시스템 콜 번호와 인자를 전달하고, 시스템 콜 인터럽트가 보내지면, 커널 모드로 전환되어 운영체제내의 시스템콜 처리 기능을 통해, 시스템콜 작업이 수행되는 것이다.

- **이번 프로젝트에서 개발할 시스템 콜에 대한 간략한 설명 (하나의 시스템 콜 당 최대 3문장으로 간략히 설명; 3문장을 넘길 정도로 길게 작성하지 말 것)**
1. halt(): 시스템 전체를 종료하는 시스템 콜로, shutdown_power_off() 함수를 호출하여 운영체제를 안전하게 종료한다.
2. exit(): 현재 실행 중인 프로세스를 종료하는 시스템 콜로, 호출 시 해당 프로세스의 종료 상태를 출력한다. 부모 프로세스는 이 상태 값을 통해 자식 프로세스가 정상적으로 종료되었는지 확인할 수 있다. 잘못된 메모리 접근이나 예외 상황에서도 이 시스템 콜을 호출해 프로세스를 안전하게 종료할때도 사용한다.
3. write(): 파일 디스크립터(fd)를 사용하여 데이터를 출력하는 시스템 콜로, 콘솔에 데이터를 기록한다. fd 값에 따라 출력 대상이 달라지며, 콘솔 출력은 fd == 1일 때 수행되는데, 현재 유저 프로그램1에서는, fd==1일때, 콘솔에 출력하는 것만 구현된 상태이다. 버퍼에 있는 데이터를 지정된 길이만큼 출력하며, 출력된 바이트 수를 반환한다.
4. read(): 콘솔에서 데이터를 입력받아 버퍼에 저장하는 시스템 콜로, 주어진 파일 디스크립터로부터 데이터를 읽어온다. fd == 0인 경우 콘솔로부터 입력을 받으며, 지정된 크기만큼 읽어들인다. 입력된 데이터는 버퍼에 저장되며, 읽은 바이트 수를 반환한다.
5. exec(): 새로운 프로그램을 실행하는 시스템 콜로, 주어진 프로그램 파일을 로드하고 실행한다. 실행된 프로그램은 현재 프로세스를 대체하며, 새로운 프로세스의 PID가 반환된다. 실행 실패 시 -1을 반환하며, 프로세스는 실패 상태로 종료된다.
- **유저 레벨에서 시스템 콜 API를 호출한 이후 커널을 거쳐 다시 유저 레벨로 돌아올 때까지 각 요소를 설명**
1. 유저 모드에서 시스템콜 인터페이스를 호출한다. ex) write()

![image](https://github.com/user-attachments/assets/1e4f54a5-0bf3-406c-8c73-eade4ad67ea6)


1. 시스템 콜이 호출되면, int 0x30 인터럽트를 사용하여 유저 모드에서 커널 모드로 전환된다.
- 시스템 호출 매크로 설명
- 인자들과 , 시스템 호출 번호를 스택에 push함
- int $0x30: 소프트웨어 인터럽트를 발생시켜 커널로 진입
- 스택 포인터(esp)를 조정하여 푸시한 인자들을 제거
- 
    
![image](https://github.com/user-attachments/assets/95940554-816b-4f2d-b799-914e1cba2e5f)

    
1. 이 인터럽트는 intr_handler()를 호출 -> 이때 시스템 콜 번호가 eax 레지스터에서 읽히게 됨
2. 시스템 콜 핸들러인 syscall_handler()가 호출됨

![image](https://github.com/user-attachments/assets/9012d55c-4d3d-4794-82e0-5cfd9f8ee515)


전달된 시스템 콜 번호를 바탕으로 적절한 시스템 콜 처리 함수를 호출 ex) write() 등등

1. 커널에서 작업을 완료한 후, syscall_handler()는 반환 값을 eax 레지스터에 저장하고, intr_exit()을 호출하여 인터럽트 리턴 방식으로 유저 모드로 돌아간다. 이때, 시스템 콜의 결과가 유저 프로그램에 반환되며, 유저 프로그램은 해당 결과 값을 받아 계속해서 실행된다.
- 이러한 양식으로 eax 레지스터에 반환값을 저장

<img width="427" alt="image" src="https://github.com/user-attachments/assets/b0f57a67-d49a-441c-918f-c582913a9aa3" />


- start_process() 하단에 존재

![image](https://github.com/user-attachments/assets/a43ed4da-0628-4c7b-bb19-9870b0a041c3)


## **추진 일정 및 개발 방법**

## C. **추진 일정**

**II. A.의 개발 범위를 포함하여 구현 내용에 대한 일정 작성**

![image](https://github.com/user-attachments/assets/932f1bc7-4fc5-4287-8937-0df376c1cc73)


1. 9월 12일 : 핀토스 세팅

깃허브 private repository로 작업 가능하게 세팅, 비주얼 스튜디오 코드로 작업 가능하게 remote server로 연결해놓음

1. 9월 20일 : additional system call을 위한 syscall4 메크로함수 먼저 구현
2. ~ 9월 21일
- arguement들을 적절하게 parsing하여 전달하는 것 구현
- additional system call을 구현하기 위해 필요한 시스템콜 인터페이스 추가, 프로토타입 추가 먼저 구현
1. ~ 9월 26일
- argument passing 까지 구현 완료
- write 시스템콜, read 시스템콜,halt 먼저 구현함
- process.c 내 user memory access 구현 완료
- > echo x 출력, stack에 쌓는 부분까지 성공
1. 25~26일 : 부모 자식 프로세스 동기화 작업을 수행할 수 있을지, 계속해서 고민함 -> 처음에는 자식 프로세스 구조체를 별도로 만들어서 동기화를 구현하려 했으나, 실패함
2. 9월 27일 ~28일
- struct thread를 활용해서 부모, 자식 프로세스 동기화 활용할 수 있도록 코드 추가
- 다른 시스템콜 작업하면서 테스트 케이스들을 확인하며, 이 부분 계속적으로 수정하였음
1. 9월 27일 : wait 시스템콜 구현
2. 9월 28일 - 1
- 계속해서 fail이 발생했던 sc-bad-sp,sc-boundary-3 둘다 passt 시킴
- syscall.c와 exeption.c에서 user meomory access 검사 보완 추가
1. 9월 28일 - 2 : exit 시스템콜 구현 완료(관련 테스트케이스 pass)
2. 9월 28일 - 3 : exec 시스템콜 구현 완성 : (exce 관련 테스트 케이스 fail하던 것 pass로)
- > 80개의 테스트 케이스 중, 35개 fail, 45개 pass 달성함
1. 9월 28일 - 4 : additional system call까지 userprog내에서 pintos 명령어로 실행 가능하도록 구현

### **개발 방법**

- **II. B.의 개발 내용을 구현하기 위해 어느 소스코드에 어떤 요소를 추가 또는 수정할 것인지 설명. (함수, 구조체 등의 구현이나 수정을 서술)**
1. Argument Passing

구현 파일: pintos/src/userprog/process.c

file name을 오픈하는 것을, strtok_r을 사용해, 첫번쨰 인자를 파싱해, full file name 대신, 첫번째 인자(ex. echo 프로그램명에 해당)를 넘기도록 수정한다.

setup_user_stack(char *file_name, void **esp) 함수를 구현하여 사용자 프로그램이 실행될 때 인자들을 스택에 적절히 설정하도록  한다. load() 함수 내에서 구현한 setup_user_stack() 함수를 호출한다.

1. User Memory Access

구현 파일

- pintos/src/userprog/syscall.c
- pintos/src/userprog/process.c
- pintos/src/userprog/exception.c

해당 파일들에, is_user_vaddr()와 pagedir_get_page() 함수를 사용해 유효한 사용자 주소인지 확인하는 로직을 추가한다.

- exception.c의 페이지 폴트가 발생했을 때 호출되는 예외 처리 함수에 해당하는 page_fault() 내에, pagedir_get_page()와 is_user_vaddr()을 사용하여 잘못된 사용자 메모리 접근을 감지할 수 있도록 한다.
- process.c에서 pagedir_get_page()와 is_user_vaddr()을 사용하여 잘못된 사용자 메모리 접근을 감지하는 is_valid_stack_pointer() 함수를 구현하여, 스택을 쌓을 때, esp로 메모리에 접근할 떄마다, 유효한 스택 주소인지 확인할 수 있도록 한다.
- syscall.c에선 시스템 콜 처리 시 , 유저의 포인터의 유효성을 검사하는 check_valid_vaddr() 함수를 추가하여, 시스템 콜 핸들러가 호출될 때 사용자 메모리에 대한 접근이 올바른지 확인하고, . check_valid_buffer() 함수는 버퍼의(인자가 여러개일경우) 모든 바이트가 유효한지, 각각 인자별로 check_valid_vaddr()를 호출하여 확인한다.
1. System Call

구현 파일

- intos/src/userprog/syscall.c
- pintos/src/userprog/process.c
- pintos/src/userprog/exception.c
- pintos/src/threads/thread.c
- pintos/src/threads/thread.h
- intos/src/userprog/syscall.c에서 syscall_handler() 함수에서 시스템 콜 번호에 따라 각 시스템 콜 함수를 호출하도록 switch-case문으로 분기를 작성한다. 이때 각각의 시스템콜 번호에 알맞게 호출될 halt(),exit(),write(),read(),exec(),그리고 추가 시스템콜을 구현한다.
- pintos/src/threads/thread.c
    - init_thread() 함수 수정 : load_sema, exit_sema, wait_sema 세마포어를 초기화
    - child_list 초기화, 부모-자식 관계를 설정하기 위해 parent 포인터를 설정
- pintos/src/threads/thread.h
    - struct thread에 load_sema, exit_sema, wait_sema 세마포어를 추가하여 부모와 자식 프로세스 간 동기화 구현
    - parent: 부모 스레드를 가리키는 포인터 추가
    - child_list: 자식 스레드들의 리스트 추가
    - child_elem: 자식 리스트에 자신을 연결하기 위한 리스트 요소 추가
    - exited, wrong_exit 플래그 추가
- pintos/src/userprog/process.c에서
    - process_wait() 함수에서 세마포어를 이용하여 자식 프로세스의 종료를 대기하도록 수정
    - start_process() 함수에서 자식 프로세스가 로드에 성공하거나 실패했을 때 부모에게 알리기 위해 세마포어 수정
    - get_child_thread_by_tid() 함수를 추가 -> 현재 스레드의 자식 스레드 중 특정 tid를 가진 스레드를 찾을 수 있도록 함
## 1. **연구 결과**
    1. **Flow Chart**
- **II. B. 개발 내용에 대한 Flow Chart를 작성**

![image](https://github.com/user-attachments/assets/6d11c242-7206-4c7f-afb7-790a1bebb532)


![image](https://github.com/user-attachments/assets/2609a076-8ded-4864-afda-01f966ff7e89)


## **제작 내용**

- **II. B. 개발 내용의 실질적인 구현에 대해 코드 관점에서 작성.**
- **구현에 있어 Pintos에 내장된 라이브러리나 자체 제작한 함수를 사용한 경우 이에 대해서도 설명.**
- **개발상 발생한 문제나 이슈가 있으면 이를 간략히 설명하고 해결책에 대해 설명.**
### 1. Argument Passing
1) process_execute() 함수 내에, 실행 파일의 이름을 추출하여, 파일 이름에서 프로그램 이름을 정확히 추출하지 못하는 문제를 해결하여, 사용자 프로그램이 제대로 실행될 수 있도록 했다.

이떄, strcspn() 함수를 사용하여 공백까지의 길이를 계산하고, strlcpy()를 사용하여 알맞은 크기의 복사본을 생성하였다.

![image](https://github.com/user-attachments/assets/a3033fda-0c05-4108-afb5-74faf20d12a6)


2) load() 함수

load() 함수 내에서 알맞은 실행 파일을 메모리에 로드할 수 있도록 첫번쨰 인자를 추출하였다.

이때 원본 문자열에다 작업하면 훼손되므로, 복사본을 생성하여 작업하였다.

file_name을 복사하고 strtok_r()을 사용하여 프로그램 이름과 인자를 분리하고, filesys_open()으로 알맞은 실행 파일을 로드하였다.  (real_file_name_copy에  해당)

또한, 첫번쨰 인자(프로그램명)뿐만 아니라, 스택에 각각의 인자들을 알맞게 쌓을 수 있도록, full_file_name_copy2를 두어, 전체 입력된 문자열의 복사본을 별도로 마련해 두었다. 이때, full_file_name_copy 자체를 사용하면, 조작 과정에서, 첫 번째 인자만 남게 되어, full_file_name_copy 말고, 이외에 full_file_name_copy_2 별도의 복사본을 생성하여, full name의 훼손을 막을 수 있었다.

![image](https://github.com/user-attachments/assets/019cb193-eb19-4061-b5f3-4d5997302e98)


또한, load() 함수 내에서 setup_stack() 이라는 빌트인 함수를 통해 스택 초기화가 성공적으로 이루어진 후에, 스택에 인자들을 알맞게 쌓을 수 있도록 직접 제작한 setupt_user_stack 함수를 호출하였다. 이때, 훼손 가능성이 없게끔 안전하게 복사한 full_file_name_copy_2 를 인자로 넘겨주었다.

![image](https://github.com/user-attachments/assets/551a1b9e-b303-46dd-89a1-5c51e405d0d3)


직접 제작한 setupt_user_stack 함수 내에서는,

최상단 (PYHBASE쪽에 해당)

---

1. y\0
2. x\0
3. echo\0
4. 패딩 (4의 배수로 맞추기 위함)
5. NULL sentinel
6. argv[2] (y)의 주소
7. argv[1] (x)의 주소
8. argv[0] (echo)의 주소
9. argv 주소
10. argc
11. fake return address

---

하단

---

위에서도 언급했던, 이와 같은 순서로 스택을 쌓는 로직을 구현했다. 이때, esp 포인터가 접근 가능한 메모리 영역을 가리키고 있는지 포인터를 이동할때마다, 직접 제작한 is_valider_stack_Pointer 함수를 통해 esp를 검사하고 만일 잘못되었으면, thread_exit()을 호출할 수 있도록 구현하였다.

1. 프로그램 이름과 인자 파싱

![image](https://github.com/user-attachments/assets/729c1d86-59d8-436b-a4dc-96054dab4e30)


strtok_r() 함수를 사용해 file_name 문자열을 공백을 기준으로 토큰화를 진행하였다.

각 토큰을 argv 배열에 순서대로 저장하고 argc도 증가시켜놓았다.

2. 인자들을 역순으로 스택에 쌓기

![image](https://github.com/user-attachments/assets/f9a0190e-1e32-4408-916b-920858a777d7)


argv[i]에 저장해놓은 인자들을 역순으로 스택에 쌓았다. 핀토스에서 스택은 상단에서 하단으로 쌓아지므로, 마지막 인자부터 저장했다. 이를 위해, 각 인자의 길이를 계산하고, 그만큼 esp를 감소시켜 공간을 확보했다. 그리고 스택에 인자에 해당하는 문자열을 복사하기 위해선, memcpy() 함수를 사용하였다. 그 후, 인자가 스택에 저장된 주소를 argv[i]에 다시 저장하여 나중에 주소 값을 사용할 수 있게 하였다.

3. 스택 패딩을 통해 4바이트 배수로 정렬

![image](https://github.com/user-attachments/assets/0f869052-747e-4d46-850c-ca93bfb3b418)



전체 인자 문자열의 길이가 4의 배수가 아닐 경우, 스택의 워드 정렬을 위해 패딩을 추가하였다.

만약, echo/0 x/0 y/0와 같이, 문자열의 길이가 4+1+1+1+1+1=9바이트라면, remainder = 9 % 4 = 1이고, padding_size = 4 - 1 = 3이 되었다. 이때에는, 3바이트의 패딩을 추가하여, 4의 배수가 될 수 있도록 로직을 구성하였다.

4. NULL Sentinel 추가

인자 주소 배열의 끝을 나타내는 NULL 포인터를 스택에 추가하였다.

<img width="321" alt="image" src="https://github.com/user-attachments/assets/d7e79e36-ae1d-4a3d-a432-838694f71faa" />


5. 각 인자의 주소를 스택에 저장

<img width="330" alt="image" src="https://github.com/user-attachments/assets/ccfeed1c-992d-42f2-bc23-53dafc8c18c6" />


스택에 저장된 인자들의 주소를 스택에 저장하였다.

6. argv의 주소 저장

argv 자체의 주소를 스택에 저장하였다.

![image](https://github.com/user-attachments/assets/6492cc1b-11d0-428c-a487-dcd989a2144b)


7. argc 저장

인자의 개수에 해당하는 argc를 스택에 저장하였다.

![image](https://github.com/user-attachments/assets/02f2c8fd-ae6b-471e-adb2-e20d544969bd)


8. 가짜 리턴 주소 저장
   
<img width="356" alt="image" src="https://github.com/user-attachments/assets/44cfb893-ecda-4774-ad1c-306790d83319" />


스택 프레임의 형태를 맞추기 위해, 가짜 리턴 주소를 저장하였다.

이를 구현하면서, 스택에 데이터를 저장할 때 esp가 유효한 사용자 메모리 영역을 벗어나면 커널 패닉이 발생했을 때가 있었다. 원래는 스택 포인터 유효성을 검사하는 부분을 추가하지 않았는데,  페이지 디렉토리에 매핑되어 있는지 확인하도록 is_valid_stack_pointer() 함수를 통해, 스택 포인터를 변경할때마다 검사하여 메모리 관련 오류들이 사라질 수 있게 되었다

.

### 2. User Memory Access

![image](https://github.com/user-attachments/assets/7b189e8d-73d2-449c-b121-e2ca4f0efcba)


핀토스의 빌트인 함수인, is_user_vaddr()를 통해,  주소가 사용자 영역에 속하는지 확인하였다. 또한, pagedir_get_page() 함수를 통해, 페이지 디렉토리에서 해당 주소가 유효한지 확인하였다. 이 두 빌트인 함수를 이용하여, check_valid_vaddr()와 check_valid_buffer()  유효성 검사 함수를 구현하였다. 이를 syscall.c 파일에서, 시스템 콜에서 사용자로부터 전달받은 포인터나 버퍼에 접근해야 하는데, 이때 해당 메모리 주소가 유효한 사용자 영역의 주소인지 확인하는 용도로 사용하였다. 즉, 이를 이용해 각 시스템 콜 핸들러에서 사용자로부터 전달받은 포인터나 버퍼에 대해 유효성 검사를 수행한 것이다. 그리고 만일, 유효하지 않은 포인터일 경우에는 프로세스가 종료될 수 있도록 exit(-1)을 호출하였다.

### 3. System Calls
- **이번 프로젝트에서 개발한 시스템 콜을 구현 관점에서 상세히 서술.**

![image](https://github.com/user-attachments/assets/74d8d83a-43f2-428e-bdf9-6a661b718730)


우선 위와 같이 syscall_handler() 함수에서 시스템 콜 번호에 따라 각 시스템 콜 함수를 호출하도록 switch - case문을 통해 구현하였다. 이때, 사용자로부터 전달된 포인터의 유효성을 검사를 각각의 콜별로 호출하여, 잘못된 메모리 접근으로부터 커널을 보호할 수 있도록 하였다.

1. halt() 시스템 콜

<img width="281" alt="image" src="https://github.com/user-attachments/assets/73e25e34-840c-448b-bbbd-add827f1ab16" />


핀토스에서 시스템을 종료하기 위해 제공되는 빌트인 함수인 shutdown_power_off() 함수를 호출하여  halt 시스템 콜을 구현하였다.

2. exit() 시스템 콜

![image](https://github.com/user-attachments/assets/7545f7d7-354d-4ddf-b2ed-e02da2c1c72e)


현재 프로세스를 종료하고, 종료 상태를 부모 프로세스에게 전달할 수 있도록 exit 시스템콜을 구현하였다.

부모 프로세스가 자식의 종료를 대기하고 있을 수 있으므로, exit_sema 세마포어를 통해 부모에게 종료를 알릴 수 있도록 했다. 그런데 이를 개발하는 과정에서, 중복된 종료 메시지가 출력되는 문제가 발생하였었다. 그리고, 종료 시차로 인해, 잘못된 메모리 접근으로 인한 커널 패닉의 문제도 발생하였었다.

이를 해결하기 위해, thread 구조체에 exited 및 wrong_exit 변수를 추가하여 상태를 관리하였다.

![image](https://github.com/user-attachments/assets/0c97d9ab-23ab-4708-9d0c-3907f39ba2d5)


이미 종료되었는데 시간차로 인해 또 종료시키려고 하여 커널 패닉도 발생하지 않게끔 싱크를 맞추는 작업이 중요했는데, 이때에도, 잘못된 메모리 접근 시 커널 패닉이 발생하지 않도록, page_fault() 핸들러에서 exit(-1)을 호출하도록 수정하였다. page_fault()내에서, 이미 exit(-1)의 종료 메시지를 출력하였으므로, 이 경우에는, exit 시스템콜 내에서 중복 종료 메시지가 출력되지 않게끔 할 필요가 있었다. 때문에,   page_fault()내에서 잘못된 메모리 접근을 원인으로 종료시킬 경우에는,   thread_current()->exited = true;로 하고, thread_current()->wrong_exit = true;로 하여, exit 시스템콜과 싱크를 맞출 수 있도록 하였다. 즉, exit 시스템콜 내에서, wrong_exit이 true인 경우에는 이미 execption.c의 page_fault 함수 내에서 종료 메시지가 출력되었을 것이므로, wrong_exit이 false인 경우에만, 종료 메시지가 출력되도록 하여 이 문제를 해결할 수 있었다.

![image](https://github.com/user-attachments/assets/873383d1-933b-4dc6-abf2-90c7955b601c)

3.  write() 시스템 콜

![image](https://github.com/user-attachments/assets/c9202654-67ca-4dc3-be27-e4bd668c73ed)


파일 디스크립터가 1인 경우, 즉 표준 출력인 경우에 버퍼의 내용을 화면에 출력하는 것을 구현하였다. 이때, 빌트인 핀토스 함수인 버퍼의 내용을 콘솔에 출력하는 putbuf()를 사용하였다. 또한, 자체 제작 함수를 통해 버퍼의 유효성을 검사하여 잘못된 메모리 접근을 방지할 수 있도록 하였다.

4. read() 시스템 콜

![image](https://github.com/user-attachments/assets/c30d0120-5202-4256-825b-820a55fd1f47)


파일 디스크립터가 0인 경우, 즉 콘솔 경우에 입력을 받아 버퍼에 저장하는 부분을 구현하였다. 이때, 핀토스의 빌트인 함수인, input_getc()를 사용하였다.

5. exec() 시스템 콜

![image](https://github.com/user-attachments/assets/5aea0f5b-aa47-4464-b607-cd75d2dc9e97)


process_execute() 함수를 호출하여 새로운 프로세스를 생성할 수 있도록 했다. 자식 프로세스가 로드될 때까지 부모 프로세스는 대기해야 했기 때문에,  자식 스레드의 load_sema 세마포어를 사용하였다. 이때, 생성된 자식 스레드가 무엇인지 알 필요가 있었기 때문에, 자식 스레드를 찾기 위한 함수인, get_child_thread_by_tid()를 구현하였다.

![image](https://github.com/user-attachments/assets/57d791a2-e54e-4c2d-b5ba-cd04c6f41cfe)


이 함수는, 부모 스레드(parent)의 자식 스레드 리스트(child_list)를 순회하해, 특정 TID(child_tid)를 가진 자식 스레드를 찾아 반환하는 역할을 한다. 이를 위해, 핀토스의 리스트 관련 내장 함수들과 구조체를 사용하였다. 리스트 순회를 위한 포인터에 해당하는 e가  리스트의 끝(list_end(&parent->child_list))에 도달할 때까지 순회하며, 각각의 순회마다, list_entry(e, struct thread, child_elem)를 통해, 리스트 요소 e로부터 자식 스레드를 얻었다. 이를 통해, 해당 t가 찾고자 하는 TID(child_tid)와 일치하는지 비교할 수 있도록 했다. 만약 일치한다면, 해당 자식 스레드의 포인터를 반환하고, 만약, 자식 스레드를 리스트에서 발견하지 못하였을 때는,  NULL을 반환할 수 있도록 하였다.

다시 exec() 시스템콜로 돌아오면, 만약, exec()에서 process_execute() 함수를 호출하여  자식 스레드가 정상적으로 생성되었다면, get_child_thread_by_tid()는 생성된 자식 스레드의 포인터를 반환해야 할 것이다. 하지만, child가 NULL이면, 자식 스레드 생성에 문제가 발생한 것이므로 -1을 반환하도록 했다. 만약 정상적으로 생성되었다면, 이렇게 반환받은 포인터를 가지고, sema_down을 통해 자식이 로드를 완료할 때까지 부모 스레드로써 대기할 수 있도록 했다. 만약, 자식 스레드가 로드를 완료하면 sema_up()을 호출하여 세마포어를 업할 것이기 때문에, 부모 스레드의 대기가 해제되고 이것 이후의 코드가 실행될 것이다. 그럼, 그 이후에, 자식 스레드의 exit_status가 -1인지 확인하여, 로드가 성공했는지의 여부를 확인했다. 그래서, 로드까지 성공했다면, exec의 모든 과정이 성공적으로 완료된 것이므로, 생성된 프로세스의 PID(pid)를 반환할 수 있도록 했다.만약 로드가 실패했다면, 자식 스레드의 포인터가 null인 경우에 -1을 반환한 것과 마찬가지로 -1을 리턴할 수 있도록 했다.

추가적으로 자식 프로세스의 로드 실패와 성공 알림은 아래와 같이, start_process 내에서 수행되었다.

![image](https://github.com/user-attachments/assets/cf43b400-1632-448a-b482-302a01adc3ef)


6. wait 시스템 콜

wait 시스템 콜은 부모 프로세스가 자식 프로세스의 종료를 대기하고, 자식 프로세스의 종료 상태를 반환받기 위해 사용되는 시스템 콜이다.

<img width="499" alt="image" src="https://github.com/user-attachments/assets/dcceda84-b3e6-42f8-b4ac-286a6bb241f7" />


먼저 get_child_thread_by_tid() 함수를 호출하여 현재 스레드(thread_current())의 자식 스레드 리스트에서 주어진 TID(tid)를 가진 자식 스레드를 찾고, 이를 반환한다. 그리고, 반환된 자식 스레드 포인터 child가 NULL이거나, child->parent가 현재 스레드가 아닌 경우에는, 해당 TID를 가진 자식 프로세스가 없거나, 이미 대기 중인 경우를 의미하기 때문에, -1을 반환하도록 했다. 이러한 예외상황이 아닌 경우에는, process_wait(tid) 함수를 호출하여 자식 프로세스의 종료를 대기할 수 있도록 했다. 적절히 대기가 가능하도록 이전에는 무한루프를 통해 자식 스레드를 대기하도록 임시방편으로 구현했던 process_wait을 아래와 같이 수정하였다.

![image](https://github.com/user-attachments/assets/24c1e5bd-0ac1-4634-8b55-55681991472c)


process_wait 함수는 특정 자식 프로세스가 종료될 때까지 현재 스레드(부모 프로세스)를 대기시키고, 자식 프로세스의 종료 상태를 반환하는 역할을 하는 함수이다. wait 시스템콜에서, process_wait 함수를 호출하면, process_wait 함수는 get_thread_by_tid 함수를 통해, 자식 스레드의 유효성을 검사한 후, 유효할 경우에, 자식 프로세스의 종료를 sema_down()을 통해 대기시킨다. 그리고 자식 프로세스가  exit 시스템콜에서 부모에게, 종료를 알리고, exit 시스템콜에서 자식이 exit에서 wait_sema를 통해, 부모가 자식을 정리할 때까지 기다리게 sema_down을 호출한다. 그러면, 자식은 부모가 종료상태를 정리할 때까지 기다리게 되는데, 이때 부모는 process_wait에서 list_remove를 통해, 자식 리스트에서 종료될 자식을 제거한 후에, sema_up을 통해, 자식에게 완전히 종료해도 된다고 알린다. 그럼, 자식 프로세스는, sema_down(&thread_current()->wait_sema);를 통해 대기하고 있다가 부모로부터 신호를 받으면  종료한다.

추가적으로 자식과 부모의 synchronization을 위해 세마포어 등의 초기화는 init_thread() 내에서 이루어졌다.

![image](https://github.com/user-attachments/assets/657659e2-d23f-4ac5-b843-98301d6ef421)


### Additional System calls

- **새로운 시스템 콜(fibonacci, max_of_four_int)을 구현하기 위해 수정하거나 작성한 코드에 대해 서술**

[lib/user/syscall.c] syscall4 추가

![image](https://github.com/user-attachments/assets/b963d05b-ba9a-4911-ae74-49f154f7121c)


[pintos/src/lib/syscall-nr.h] : enum에 추가 시스템콜 번호 추가,

![image](https://github.com/user-attachments/assets/6034a242-09ea-4a5f-a13b-49d7476fa8f4)


[pintos/src/lib/user/syscall.h] : 추가 시스템콜 프로토타입 추가

![image](https://github.com/user-attachments/assets/b2cf85d2-0bb7-44a1-a08a-c2352a9afb79)


[pintos/src/lib/user/syscall.c] : 추가 시스템콜 인터페이스 추가

![image](https://github.com/user-attachments/assets/75c3260d-427d-45c5-93f3-67a4bc49ae2e)

[pintos/src/userprog/syscall.c] : 피보나치, max값 구하는 시스템콜 구현

![image](https://github.com/user-attachments/assets/adbce1aa-c6cc-4f49-858f-9ab75547ce32)


[pintos/src/userprog/syscall.c] : 피보나치, max값 구하는 시스템콜 호출 시스템콜 핸들러에 추가

![image](https://github.com/user-attachments/assets/a95f637d-d861-4656-b9c8-2dd9ff7ae86e)


[pintos/src/examples/Makefile]: additional 유저 프로그램 실행 위한 Makefile 수정

![image](https://github.com/user-attachments/assets/d54e28ff-9e3b-436a-9b46-736604cd5a27)


[pintos/src/examples/additional] : additional 유저 프로그램 구현

![image](https://github.com/user-attachments/assets/cb0abbe1-47fc-49df-9510-f69bf6894d24)


##  **시험 및 평가 내용**
- **fibonacci 및 max_of_four_int 시스템 콜 수행 결과를 캡처하여 첨부.**

![image](https://github.com/user-attachments/assets/7fd79437-5141-4055-a65b-e26e9fa721b1)


![image](https://github.com/user-attachments/assets/52fe2f5d-82ab-4bfc-a8f3-13ec8efde2a9)


- **추가) 테스트 케이스들 실행 결과**

![image](https://github.com/user-attachments/assets/b46e9de9-fcbc-4ae2-8ad0-85c94126ad95)


-----------------------------------------------------------------------------------------------------------------------


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



