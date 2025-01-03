# Pintos Project 4: Virtual Memory
 
담당 교수 :김영재
이름 / 학번 :이지인
개발 기간 :12월 9일 - 18일


## 개발 목표
해당 프로젝트에서 구현할 내용을 간략히 서술
이번 프로젝트의 개발 목표는 Pintos OS에 가상 메모리 시스템을 구현하는 것이다. 이를 통해 페이지 테이블과 페이지 폴트 핸들러를 구축하고, 디스크 스왑 메커니즘을 도입하며, 스택 growth 또한 구현해야 한다. 

## 개발 범위 및 내용


### 개발 범위
아래 각 항목 개발의 필요성 또는 개발 시 기대되는 결과를 간략히 서술

#### Page Table & Page Fault Handler
페이지 테이블과 페이지 폴트 핸들러를 구현하면, 사용자 프로그램이 접근하려는 가상 주소가 실제 물리 메모리에 매핑되어 있는지 확인이 가능하다. 또한, 페이지 펄트 핸들러에서, 페이지 펄트가 발생할 경우에, 스와핑을 통해, 해당 페이지를 복원하는 등의 후속 처리를 진행할 수 있게 된다. 

#### Disk Swap
disk swap을 구현함으로써, main meory 즉 물리 메모리가 부족할 때, 사용하지 않는 페이지를 디스크로 옮겨서 메모리를 확보한다. 

#### Stack Growth
이는, 유저 프로그램이 필요한 만큼의 stack space를 동적으로 할당받을 수 있는 것이다.즉, 스택 공간을 더 할당해주어, 스택 오버플로우와 같은 문제가 발생하지 않도록 해준다고 할 수 있다. 

### 개발 내용
아래 항목의 내용만 서술
#### 1. Page fault가 발생하는 이유와 이를 handling하는 전반적인 과정을 서술
페이지 폴트는 사용자 프로그램이 현재 물리 메모리에 매핑되지 않은 가상 주소에 접근하려 할 때 발생한다고 할 수 있다. page fault를 handling 하는 과정을 설명해보자면, 먼저 첫번쨰로는 페이지 펄트의 원인을 분석하는 것이다. 페이지 폴트의 원인이 페이지가 존재하지 않음(not_present)인지, 권한 문제(write 시도 등)인지를 확인하는 것이 필요하다. → 다음으로는, spt 즉, suppllemntal page table을 조회하는 것인데, 잘못된 접근 주소를 보조 페이지 테이블에서 조회하여 해당 페이지의 존재 여부와 속성을 확인하는 것이라고 할 수 있다. 다음으로는, 페이지가 SPT에 존재하면, 필요한 물리 프레임을 할당하고 페이지 디렉토리에 매핑한 후, 페이지 데이터를 파일 또는 스왑 공간에서 로드해야한다. 만약, 반대로, 페이지가 SPT에 존재하지 않으면, 스택 확장을 시도하여 필요한 페이지 엔트리를 추가하고 페이지를 로드해야 할 것이다. 그다음으로 만약 페이지가 성공적으로 로드되었다면, 페이지 디렉토리를 업데이트해서, 유저 프로그램이 해당 페이지에 접근할 수 있게 한다. 그리고 마지막으로는 이러한 페이지 로딩과정에서 문제가 발생하면 해당 프로세스를 종료시킬 수 있어야 할 것이다. 

#### 2. Disk swap 발생 시 사용한 page replacement algorithm에 대해 서술
디스크 스왑을 구현하기 위해 Second Chance 알고리즘을 사용해야 한다. 해당 알고리즘은 일반적인 fifo 알고리즘과 달리, 페이지의  accesssed 비트를 확인해서,  자주 사용되는 페이지에 대한 교체를 방지하고, 자주 사용되지 않은 페이지를 교체의 대상이 되도록 한다. 즉, frame 테이블에서 페이지를 순차적으로 탐색하고, 현재 페이지의 accessed 비트를 검사하고, 해당 비트가 1이면 두번쨰 기회를 주고, 만약 0이면, 해당 페이지를 Swap out의 대상으로 삼는다. 즉,  선택된 페이지를 디스크의 스왑 공간으로 옮기고, 프레임을 재할당하고 그 후, 새로운 페이지를 로드하고, 페이지 디렉토리를 업데이트해야 할 것이다. 

#### 3. Stack growth 구현 시 stack 확장 여부를 판단할 수 있는 방법에 대해 서술
이를 판단하기 위해선, 체크해야할 몇가지의 조건들이 존재한다고 할 수 있다. 
접근 주소와 스택 포인터의 거리: 페이지 폴트가 발생한 주소가 현재 스택 포인터(esp)보다 약간 낮은 주소(예: 32바이트 이내)인지 확인한다. 이는 스택의 정상적인 확장 범위 내에서의 접근을 의미한다.
스택의 최대 크기 제한: 스택이 최대 크기를 초과하지 않는지 확인한다.
사용자 주소 validity: fault 주소가 사용자 영역에 속하는지 확인해서, 커널 영역의 잘못된 접근을 방지한다.
이미 매핑된 페이지인지 확인: 폴트 주소의 페이지가 이미 페이지 디렉토리에 매핑되어 있는지 확인하여 중복 매핑을 방지한다.



## 추진 일정 및 개발 방법
### 추진 일정
II. A. 개발 범위를 포함하여 구현 내용에 대한 일정 작성
stack growth를 8-18까지 구현시도 성공하였고, 나머지 disk swap과 pagefault도 구현시도하여서 코드는 작성하였으나, 실제 테스트 케이스 패스에는 실패하였습니다.

![image](https://github.com/user-attachments/assets/951c1c5f-862b-45a3-a619-ca5f33ae9467)


### 개발 방법
II. B.의 개발 내용을 구현하기 위해 각각에 대해 다음 사항들을 포함하여 설명

#### Page Table & Page Fault Handler
(아래의 사항들 중 일부는 구현한 코드에 포함되어있지 않습니다.(미완성이여서, stack growth 테스트 케이스는 통과하는 이전의 코드로 제출했습니다. 하지만, 아래와 같이, 구현하기 위해 소스코드/자료구조/함수를 추가하면 될 것입니다.)stack growth 이외의부분의 개발에는 실패한 상황입니다. 때문에, 코드도, 18일까지 개발한 버전으로 과제 제출하였고, 일정도 해당 부분까지입니다.)

##### 수정해야하는 소스코드:
- userprog/exception.c: 페이지 폴트 핸들러 구현
- vm/page.c 및 vm/page.h: 페이지 테이블 관리 구조체 및 함수 구현
- vm/frame.c 및 vm/frame.h: 프레임 할당 및 관리 함수 수정

##### 수정하거나 추가해야 하는 자료구조:
- Supplemental Page Table (SPT) 구조체: 해시 테이블을 이용하여 각 프로세스의 페이지 정보를 관리
- Page Table Entry (PTE): 각 가상 페이지에 대한 속성 정보 (타입, 읽기/쓰기 권한, 스왑 인덱스 등)

##### 수정하거나 추가해야 하는 함수:
- page_fault_handler(): 페이지 폴트 발생 시 호출되는 핸들러 함수
- page_load(): 페이지를 물리 메모리에 로드하는 함수
- page_lookup(): SPT에서 특정 가상 주소의 페이지 엔트리를 찾는 함수
- try_grow_stack(): 스택 확장을 시도하는 함수

#### Disk Swap
##### 수정해야하는 소스코드:
- vm/swap.c 및 vm/swap.h: 스왑 공간 관리 함수 구현
- vm/frame.c: 페이지 교체 알고리즘 구현 및 스왑 아웃/인 로직 추가

##### 수정하거나 추가해야 하는 자료구조:
- Swap Table: 스왑 공간의 사용 여부를 관리하는 데이터 구조
- Frame Table Entry (FTE): 각 물리 프레임의 상태 및 소유 정보를 관리

##### 수정하거나 추가해야 하는 함수:
- swap_init(): 스왑 공간 초기화 함수
- swap_out(): 페이지를 스왑 공간으로 내보내는 함수
- swap_in(): 스왑 공간에서 페이지를 복원하는 함수
- pick_victim_frame(): 페이지 교체 시 희생할 프레임을 선택하는 함수

#### Stack Growth
##### 수정해야하는 소스코드:
- userprog/exception.c: 페이지 폴트 핸들러 내 스택 확장 로직 추가
- vm/page.c 및 vm/page.h: 스택 페이지 엔트리 관리 함수 구현

##### 수정하거나 추가해야 하는 자료구조:
- Stack Page Entry: 스택 영역에 대한 페이지 엔트리 정보

##### 수정하거나 추가해야 하는 함수:
- try_grow_stack(): 스택 확장 조건 검사 및 SPT에 엔트리 추가
- expand_stack(): 실제로 스택을 확장하여 페이지를 로드하는 함수


## 연구 결과
### Flow Chart
II. B. 개발 내용의 각 항목에 대하여 Flow Chart 작성
#### Page Fault Handler Flow Chart
![image](https://github.com/user-attachments/assets/e5dc09b6-1d72-49ab-b8fb-be2f6d7818b3)

#### stack growth 플로우 차트

![image](https://github.com/user-attachments/assets/4bd3cdcc-b886-420d-94a5-30ac6aaafa44)


#### disk swap (second chance clock algorithm)
![image](https://github.com/user-attachments/assets/5ce984ba-db65-4c10-9af5-71284e3fcf80)


## 제작 내용
II. B. 개발 내용의 각 항목에 대하여 실질적으로 구현한 코드의 관점에서 작성 (구현 내용, 알고리즘 등을 명확히 서술할 것)

### 1. 스택 확장 기능 
#### 페이지 펄트 함수
![image](https://github.com/user-attachments/assets/586b7af0-edda-4b27-95e4-4823f6fd91c6)

레지스터에서 폴트가 발생한 가상 주소(fault_addr)를 가져옴 ->  is_user_vaddr(fault_addr) 함수를 사용하여 폴트 주소가 사용자 영역에 속하는지 확인 -> 페이지가 존재하지 않는 경우(not_present), try_grow_stack 함수를 호출하여 스택 확장을 시도 -> 실패 시, 프로세스 종료

#### try grow stack 함수
![image](https://github.com/user-attachments/assets/e9ba564e-2e40-446d-a66f-f516a4ce468e)

거리 계산(-> 너무 멀리 떨어진 주소는 스택 확장을 허용x)->  fault_addr가 스택의 최대 크기(MAX_STACK_SIZE)를 초과하지 않는지 확인 -> fault_addr가 이미 페이지 디렉토리에 매핑되어 있는지 확인 -> palloc_get_page 함수를 사용하여 새로운 페이지를 할당 + pagedir_set_page 함수를 통해 페이지 디렉토리에 매핑

### 2. Supplemental Page Table 관리
![image](https://github.com/user-attachments/assets/2d729c2e-8f05-46c2-83e9-6fa59e26a40f)

각 프로세스의 가상 주소 공간을 관리하면서 페이지의 상태와 속성을 저장하기 위해 추가했다. hash table을 기반으로 구현하였다.  supplemental_page_table.c 코드에 SPT 초기화하고, destroy하는 등 이 자료구조를 활용하기 위한 각종 함수를 구현하였다.

![image](https://github.com/user-attachments/assets/1c010618-93f7-4f18-b1a8-99c14623ebfe)

#### page load 함수

![image](https://github.com/user-attachments/assets/3ef57c20-d04a-4572-8b85-0b757a026217)

보조 페이지 테이블을 관리하며, 페이지 로드를 담당하는 page_load 함수도 해당 코드 파일 내에 구현하였다. 이는 필요한 페이지를 물리 메모리에 로드하고 매핑하는 용도를 위해 구현했다. 

#### swap.c
![image](https://github.com/user-attachments/assets/2ec1ba00-6911-4539-938f-bc5dd12ff095)

스왑 디스크 블록을 획득하고, 스왑 공간의 크기를 계산하여 비트맵(swap_map)을 초기화화하느 함수와, 스왑 슬롯을 할당하고, 해당 슬롯에 페이지 데이터를 디스크에 기록하는 함수, 그리고 스왑 슬롯을 할당하고, 해당 슬롯에 페이지 데이터를 디스크에 기록하는 swap in 함수까지 구현하였다. 


### 구현에 있어 Pintos에 내장된 라이브러리나 자체 제작한 함수를 사용한 경우 이에 대해서도 설명
- is_user_vaddr(void *addr): 주어진 주소가 사용자 영역에 속하는지 확인하는 함수로, 유효한 사용자 주소인지 검증하는 데 사용된다.


- pagedir_get_page(uint32_t *pd, const void *uaddr): 페이지 디렉토리에서 주어진 가상 주소에 매핑된 물리 주소를 반환하는 함수로, 페이지의 존재 여부를 확인하는 데 사용된다.


- pg_round_down(const void *addr): 주어진 주소를 페이지 경계로 내림하여 페이지 단위 주소를 얻는 함수.


- palloc_get_page(enum palloc_flags flags): 물리 메모리에서 새로운 페이지를 할당하는 함수로, 스택 확장 시 새로운 프레임을 할당하는 데 사용된다.


- pagedir_set_page(uint32_t *pd, void *upage, void *kpage, bool writable): 페이지 디렉토리에 가상 주소와 물리 주소를 매핑하는 함수로, 새로운 스택 페이지를 메모리에 매핑하는 데 사용된다.


- thread_current(): 현재 실행 중인 스레드를 반환하는 함수로, 현재 프로세스의 스레드 정보를 얻는 데 사용된다.


- thread_exit(): 현재 스레드를 종료하는 함수로, 비정상적인 페이지 접근 시 프로세스를 잘 종료하는 데 사용된다.
lock 관련 함수도 많이 사용하였다. 


## 개발 중 발생한 문제나 이슈가 있으면 이를 간략히 설명하고 해결한 방식에 대해 설명
![image](https://github.com/user-attachments/assets/67c45236-57a0-4c2e-b18c-fc36857fb416)

이전에 유저 프로그램 프로젝트를 개발 하였을때는, exit시, 형식이 프로그램명 : exit(-1)이 아니라, syscall.c 내 exit의 특정 조건에서는 프로그램명 : ext(-1) 형식으로 어디선 그냥 exit 형식으로.. 등등 다양하게 출력되도록 구현하였었다. 하지만, 이로 인해 테스트 케이스가 실패하여 테스트 케이스의 형식을 맞추기 위해, 무조건적으로 위와 같이 출력되도록 수정하였다. 

<img width="451" alt="image" src="https://github.com/user-attachments/assets/c5c98722-933b-4982-8ea5-02735aea1ebd" />

<img width="375" alt="image" src="https://github.com/user-attachments/assets/af93083a-dfad-4fe3-835a-b6f3ba303f06" />


## 시험 및 평가 내용
(채점 대상 테스트 케이스에 해당하는) make check 수행 결과를 캡처하여 첨부

<img width="386" alt="image" src="https://github.com/user-attachments/assets/98e4a05b-316c-4df2-b464-87d570ab59f4" />




