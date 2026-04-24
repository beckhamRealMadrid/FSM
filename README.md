# 🛠️ FSM (Finite State Machine System)

![C++](https://img.shields.io/badge/C%2B%2B-High%20Performance-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

---

## 🚩 프로젝트 개요

멀티스레드 환경에서 동작하는 게임 서버 AI를  
명확하고 확장 가능한 구조로 관리하기 위해  
**FSM(Finite State Machine)** 기반 상태 관리 시스템을 설계 및 구현한 프로젝트입니다.

---

### 🎯 설계 목표

- AI의 행동 흐름을 **상태(State) 단위로 분리**
- 상태 간 전이를 **트리거(Trigger) 기반으로 정의**
- 상태 변화 흐름을 **명확하게 표현**

---

### 🧠 핵심 구조

- 각 상태는 **독립적인 객체**로 구성
- 전이 테이블을 통해 **가능한 상태 변경을 명시적으로 관리**
- **OnEnter / OnUpdate 분리**를 통해 상태별 행동 로직을 구조화

---

### 🔄 상태 전이 방식

- 트리거 발생 시 전이 테이블을 기반으로 다음 상태 결정
- 상태 전이는 명확한 조건 하에서만 수행되어 흐름 추적이 용이

---

### 🧵 멀티스레드 안정성

- 캐릭터 단위에 Lock 적용
- 상태 전이 시 동기화 처리로 **race condition 방지**
- 여러 스레드 환경에서도 **일관된 상태 유지**

---

### ✅ 기대 효과

- 상태 흐름의 **가독성 향상**
- AI 로직의 **확장성 확보**
- 유지보수 시 **영향 범위 최소화**
- 멀티스레드 환경에서도 **안정적인 동작 보장**

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
