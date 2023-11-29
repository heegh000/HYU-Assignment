# 데이터베이스시스템및응용

2019년 2학기에 수강한 데이터베이스시스템및응용 과제입니다.

Project 1 (SQL Practice)
---

- Pokemn Database에 대해 SQL Query를 작성하여 문제를 푸는 과제입니다.
- DBMS는 MySQL을 사용했습니다.

Project 2 (Disk-based B+tree)
---

- 주어진 B+tree([http://www.amittai.com/prose/bpt.c](http://www.amittai.com/prose/bpt.c))를 분석하고, B+tree를 In-memory가 아니라 Disk-Based로 구현하는 과제입니다.
- 즉 data file을 읽어 insert, open, find, delete 연산을 file 기반으로 가능하게 만드는 것이 목표입니다.
- 또한 Merge의 큰 disk I/O을 줄이기 위해,  branching factor에 상관없이 해당 페이지의 모든 키가 삭제되기 전까지 merge를 하지 않는 Delayed merge를 구현하는 과제입니다.
  
- data file에 쓰여진 format(header_page_t 구조체, temp_page_t 구조체)에 맞춰 data file을 읽은 후, page_t 구조체 변수 세팅하고, 이를 바탕으로 B+tree 연산을 진행하였습니다.
- 또한 data file에 쓸 때에도 page_t 구조체 변수를 주어진 format으로 변환하여 disk에 쓸 수 있게 구현하였습니다.
- free page list를 구현하여 free page를 관리하고 있습니다.

Project 3 (Buffer management)
---

- Proejct 2에서 구현한 Disk-based B+tree에서 In-memory buffer를 구현하는 과제입니다.
  
- db를 시작할 때 버퍼의 갯수를 받아 미리 공간을 할당합니다.
- 원하는 page에 접근할 때 버퍼에 그 page가 있는지 확인한 후 버퍼에 비어있는 공간을 찾아 In-memory에 할당합니다.
- 만약 버퍼에 저장되어 있는 페이지가 변경되었다면 dirty flag를 세워서 나중에 다시 disk에 쓸 때 write를 수행하도록 하였습니다.
- 만약 버퍼에 공간이 없다면 doubly linked list를 구현하여 LRU 정책을 통해 disk에 write하여 버퍼 공간을 비웁니다.

Project 4 (Join table)
---
- Project 3에서 구현한 In-memory buffer를 가진 Disk-based B+tree에 Join 연산을 추가하는 과제입니다.
  
- 인자로 전달받은 2개의 테이블의 리프 노드 page를 각각 돌면서, key값이 같은 record들을 새로운 dafa file에 작성하도록 하였습니다..

Project 5 (Concurrency Control)
---
- Buffer Pool, Buffer page, Transation, Lock table에 대한 Latch를 pthread_mutex를 이용해 구현합니다.
- Transaction manager를 구현하여 Transaction을 가능하도록 구현하는 과제입니다.
- Record lock을 만들어 update, find시 lock을 잡고 연산을 가능하도록 만들되, 만약 deadlock이 발생한다면 Transation을 abort해야 합니다.
