# 운영체제

2022년 1학기에 수강한 운영체제 과제입니다.

xv6에 scheduler, LWP(thread), reader/writer lock, double/triple inode(파일 최대 크기 확장), thread safe file read/write(pread, pwrite)를 구현하는 과제입니다.



Project 1
---
- 기존 xv6의 Round Robin Scheduler에서 Multi-Level Feedback Queue와 Stride Scheduler을 결합한 새로운 scheduler를 구현하는 과제입니다.

- MLFQ + Stride 구현 전 디자인: [Milestone 1](https://github.com/heegh000/HYU-Assignment/wiki/Project1-Milestone1)
- MLFQ + Stride 구현 후 리포트: [Milestone 2](https://github.com/heegh000/HYU-Assignment/wiki/Project1-Milestone2)

Project 2
---
- Project 1에서 구현한 xv6를 바탕으로 Light Weight Process 즉 thread를 구현하는 과제입니다.

- thread 구현 전 디자인: [Milestone 1](https://github.com/heegh000/HYU-Assignment/wiki/Project2-Milestone1)
- thread system call 구현 후 리포트: [Milestone 2](https://github.com/heegh000/HYU-Assignment/wiki/Project2-Milestone2)
- 기존 xv6와의 호환 구현 후 리포트: [Milestone 3](https://github.com/heegh000/HYU-Assignment/wiki/Project2-Milestone3)

Proejct 3
---
- Project 2에서 구현한 xv6에 semaphore를 구현하고, 이를 통해 reader/writer lock을 구현하는 과제입니다.

- semaphore, reader/writer lock 구현 전 디자인, 구현 후 리포트: [Milestone 1](https://github.com/heegh000/HYU-Assignment/wiki/Project3)

Project 4
---
- Project 3에서 구현한 xv6에서, indirect inode를 double indirect inode, triple indirect node까지 확장하여 약 1GB까지 파일을 저장할 수 있게 구현하는 과제입니다.
- 또한 thread가 race condition 걱정없이 file을 read, write할 수 있는 system call을 구현하는 과제입니다.

- double/triple inode(파일 최대 크기 확장) 리포트: [Milestone 1](https://github.com/heegh000/HYU-Assignment/wiki/Project4-Milestone1)
- thread safe file read/write(pread, pwrite) 리포트: [Milestone 2](https://github.com/heegh000/HYU-Assignment/wiki/Project4-Milestone2)
