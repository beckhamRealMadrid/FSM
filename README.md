# 🛠️ FSM (Finite State Machine System)

![C++](https://img.shields.io/badge/C%2B%2B-High%20Performance-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

---

## 🚩 프로젝트 개요

멀티스레드 환경에서 동작하는 게임 서버 AI 로직을 정리하기 위해  
FSM(Finite State Machine) 구조를 직접 구현한 프로젝트입니다.

기존에는 상태별 로직이 여기저기 흩어져 있고,  
조건 분기도 많아서 흐름을 따라가기 어렵고 유지보수도 힘든 상태였습니다.

특히 여러 스레드에서 동시에 상태를 건드리는 상황에서  
상태가 꼬이거나 예외 케이스가 계속 늘어나는 문제가 있었습니다.

이 문제를 해결하기 위해  
**상태를 객체 단위로 분리하고, 트리거 기반 전이 구조로 재구성**했습니다.

---

## 🧱 구성 요소

### CFsm
FSM 전체를 관리하는 클래스  
상태 객체들을 보관하고 초기화(Open) 역할을 담당합니다.

---

### CFsmState
각 상태(Idle, Combat 등)를 담당하는 클래스  

- 상태 진입: `OnEnter()`  
- 상태 업데이트: `OnUpdate()`  
- 상태 전이: 트리거 기반 테이블  

상태마다 전이 조건을 따로 정의할 수 있게 되어 있습니다.

---

### CChar
FSM을 실제로 사용하는 객체  

- 현재 상태 보관  
- 상태 전이 요청 (Walk, Hit, Die 등)  
- 상태 업데이트 처리  

---

### CFsmMgr
FSM 인스턴스를 관리하는 싱글톤  

Passive / Active 타입 FSM을 생성하고 제공하는 역할입니다.

---

### CLocker
멀티스레드 환경에서 사용하는 Lock 클래스  

- CriticalSection 기반  
- RAII 방식으로 Lock/Unlock 자동 처리  

---

## 🔄 상태 흐름

```

Idle → Stroll → Combat → Dead → Ghost

```

- Idle: 대기 상태  
- Stroll: 배회  
- Combat: 전투  
- Dead: 사망  
- Ghost: 유령 상태  

---

## ⚙️ 구현 포인트

- 상태별 클래스로 분리해서 책임을 명확하게 나눔  
- 트리거 기반 전이 구조로 분기 로직 단순화  
- RAII Lock을 사용해서 멀티스레드 접근 제어  
- 상태 시퀀스를 두어 지연된 이벤트 무효화 처리  
- Passive / Active FSM을 분리해서 AI 유형 대응  

---

## 📂 디렉토리 구조

```

/FSM
├── CChar.h / CChar.cpp
├── CFsm.h / CFsm.cpp
├── CFsmState.h / CFsmState.cpp
├── CFsmMgr.h / CFsmMgr.cpp
├── CLocker.h / CLocker.cpp
└── README.md

````

---

## 🧪 사용 예시

```cpp
CFsmMgr::This().Open();

CChar ch;
ch.Spawn(TAiType::Passive);

ch.Walk();       // Idle → Stroll
ch.Hit(nullptr, 50); // Combat 전이
ch.Die();        // Dead 상태 전이
````

---

## 🛡️ 참고 사항

* 상태 전이는 반드시 FSM을 통해서만 처리하도록 구성
* CChar 내부 상태 접근 시 Lock 전제
* TaskScheduler 연결 전까지는 Update 수동 호출 필요

---

## 📜 License

MIT License © 2025
