#pragma once

using MSG_TYPE = DWORD;
class CChar;

struct OnParentMsg
{
	OnParentMsg() {}
	OnParentMsg(MSG_TYPE _dwMsgType) : dwMsgType(_dwMsgType) {}
	MSG_TYPE dwMsgType;
};

struct OnTaskFsmUpdate : public OnParentMsg
{
	CChar*	mChar;
	DWORD	mSeq;
	INT		mStateID;
	DWORD	mVal;
};

constexpr int TASK_PROCESS_INTERVAL = 100;	// 작업 실행 간격 (밀리초 단위)
constexpr int IDLE_TICK_INTERVAL_PASSIVE = (10 * 1000) / TASK_PROCESS_INTERVAL; // Idle 주기(수동적)
constexpr int IDLE_TICK_INTERVAL_ACTIVE = (2 * 1000) / TASK_PROCESS_INTERVAL;	// Idle 주기(능동적)
constexpr int MOVE_TICK_INTERVAL = (1 * 1000) / TASK_PROCESS_INTERVAL;			// 이동(걷기, 추격 등) 주기
constexpr int DIE_TICK_INTERVAL = (30 * 1000) / TASK_PROCESS_INTERVAL;			// Die상태로 Map에 존재하는 시간

namespace Fsm
{
	template <typename E>
	constexpr std::underlying_type_t<E> ToIndex(E e) noexcept
	{
		static_assert(std::is_enum_v<E>);
		return static_cast<std::underlying_type_t<E>>(e);
	}

	enum class EFsmTrigger : int
	{
		Invalid = -1,
		Respawn,		// 죽은 뒤 다시 등장
		Stroll,         // 목적 없이 이동
		Aggro,			// 전투 시작
		Disappear,		// 목표 사라짐
		Die,            // 사망
		Ghost,          // 유령 상태로 전환됨
		Max
	};

	enum class EFsmState : int
	{
		Invalid = -1,
		Idle,			// 대기
		Stroll,         // 산책/배회
		Combat,         // 전투 중
		Dead,           // 죽은 상태
		Ghost,          // 사망 후 유령 상태
		Max
	};

	enum class EFlag
	{
		Battle,
		Max
	};
}

using TTriggerID = Fsm::EFsmTrigger;
using TStateID = Fsm::EFsmState;
using TFlag = Fsm::EFlag;
using Fsm::ToIndex;

class CChar;
class CFsm;

class CFsmState
{
private:
	using TStateFlag = std::bitset<ToIndex(TFlag::Max)>;
	using TStateTable = std::vector<TStateID>;
public:
	CFsmState(TStateID pStateID, bool pIsBattle);
	virtual ~CFsmState();
	CFsmState(const CFsmState&) = delete;
	CFsmState& operator=(const CFsmState&) = delete;
public:
	DWORD Open(DWORD pTriggerCnt, const CFsm* pFsm);
	void Close();
	bool IsOpen() const;
	TStateID GetStateID() const;
	bool IsBattle() const;
	const CFsmState* Transit(TTriggerID pTriggerID, CChar* pChar) const;
	bool CanTransit(TTriggerID pTriggerID) const;
	DWORD PostStateUpdate(DWORD pAfter, CChar* pChar, DWORD pVal = 0) const;	
	DWORD GetTriggerCnt() const;
	virtual bool IsTransitable(CChar* pChar, TTriggerID pTriggerID) const;
	virtual	void OnEnter(CChar* pChar) const = 0;
	virtual void OnUpdate(CChar* pChar, DWORD pVal) const = 0;
protected:
	void _RegisterTransitions(std::initializer_list<std::pair<TTriggerID, TStateID>> pList);
	void _RegisterTransit(TTriggerID pTriggerID, TStateID pStateID);	
	virtual	void _BuildTransitionTable() = 0;
private:
	void __Dtor(VOID);
	void __ResetAttr(VOID);
private:
	const CFsm* __mFsm; // 이 상태가 속한 FSM
	const TStateID __mStateID; // 현재 상태 ID
	TStateFlag __mFlag; // 상태 속성 플래그 (전투 여부 등)
	TStateTable __mTransitionTable;	// 트리거 -> 상태 전환 테이블
};

class CIdle : public CFsmState
{
public:
	CIdle() : CFsmState(TStateID::Idle, false) {}
public:
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
protected:
	void _BuildTransitionTable() override;
};

class CStroll : public CFsmState
{
public:
	CStroll() : CFsmState(TStateID::Stroll, false) {}
public:
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
protected:
	void _BuildTransitionTable() override;
};

class CCombat : public CFsmState
{
public:
	CCombat() : CFsmState(TStateID::Combat, true) {}
public:
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
protected:
	void _BuildTransitionTable() override;
};

class CDead : public CFsmState
{
public:
	CDead() : CFsmState(TStateID::Dead, true) {}
public:
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
	bool IsTransitable(CChar* pChar, TTriggerID pTriggerID) const override;
protected:
	void _BuildTransitionTable() override;
};

class CGhost : public CFsmState
{
public:
	CGhost() : CFsmState(TStateID::Ghost, true) {}
public:	
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
	bool IsTransitable(CChar* pChar, TTriggerID pTriggerID) const override;
private:
	void _BuildTransitionTable() override;
};

class CActiveIdle : public CIdle
{
public:
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
};

class CActiveStroll : public CStroll
{
public:
	void OnEnter(CChar* pChar) const override;
	void OnUpdate(CChar* pChar, DWORD pVal) const override;
};