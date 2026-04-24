#include "pch.h"
#include "CFsmState.h"
#include "CFsm.h"
#include "CChar.h"

CFsmState::CFsmState(TStateID pStateID, bool pIsBattle) : __mStateID(pStateID)
{
	assert(TStateID::Invalid != pStateID);

	__ResetAttr();

	if (pIsBattle)
	{
		__mFlag.set(ToIndex(TFlag::Battle));
	}
}

CFsmState::~CFsmState()
{
	__Dtor();
}

void CFsmState::__ResetAttr(VOID)
{
	__mFsm = nullptr;
	__mFlag.reset();
}

void CFsmState::__Dtor(VOID)
{

}

bool CFsmState::IsOpen() const
{
	return !__mTransitionTable.empty();
}

TStateID CFsmState::GetStateID() const
{
	return __mStateID;
}

bool CFsmState::IsBattle() const
{
	return __mFlag.test(ToIndex(TFlag::Battle));
}

DWORD CFsmState::GetTriggerCnt() const
{
	return ToIndex(TTriggerID::Max);
}

DWORD CFsmState::Open(DWORD pTriggerCnt, const CFsm* pFsm)
{
	assert(!IsOpen());
	assert(0 < pTriggerCnt);
	assert(nullptr != pFsm);

	__mFsm = pFsm;

	try
	{
		__mTransitionTable.clear();
		__mTransitionTable.resize(pTriggerCnt, TStateID::Invalid);
	}
	catch (const std::bad_alloc&)
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	
	_BuildTransitionTable();
	
	return 0;
}

void CFsmState::Close()
{
	__Dtor();
	__mTransitionTable.clear();
	__ResetAttr();
}

void CFsmState::_RegisterTransitions(std::initializer_list<std::pair<TTriggerID, TStateID>> pList)
{
	for (const auto&[aTrigger, aState] : pList)
	{
		_RegisterTransit(aTrigger, aState);
	}
}

void CFsmState::_RegisterTransit(TTriggerID pTriggerID, TStateID pStateID)
{
	assert(IsOpen());
	assert((0 <= ToIndex(pTriggerID)) && (ToIndex(pTriggerID) < __mTransitionTable.size()));
	assert(TStateID::Invalid == __mTransitionTable[ToIndex(pTriggerID)]);
	assert((0 <= ToIndex(pStateID)) && (pStateID != TStateID::Invalid));

	__mTransitionTable[ToIndex(pTriggerID)] = pStateID;
}

bool CFsmState::CanTransit(TTriggerID pTriggerID) const
{
	assert(IsOpen());
	assert((0 <= ToIndex(pTriggerID)) && (ToIndex(pTriggerID) < __mTransitionTable.size()));

	return (__mTransitionTable[ToIndex(pTriggerID)] != TStateID::Invalid);
}

bool CFsmState::IsTransitable(CChar* pChar, TTriggerID pTriggerID) const
{
	bool aIsAlive = pChar->IsAlive();

	switch (pTriggerID)
	{
		case TTriggerID::Stroll:
		case TTriggerID::Aggro:
		case TTriggerID::Disappear:
			return aIsAlive;

		case TTriggerID::Die:
			return !aIsAlive;

		default:
			return false;
	}
}

const CFsmState* CFsmState::Transit(TTriggerID pTriggerID, CChar* pChar) const
{
	assert(IsOpen());
	assert(pChar != nullptr);
	assert((0 <= ToIndex(pTriggerID)) && (ToIndex(pTriggerID) < __mTransitionTable.size()));

	const CFsmState* aNextState = nullptr;
	{
		CChar::TLocker aLock(pChar);
		const CFsmState* aCurState = pChar->GetState();
		if (this != aCurState)
		{
			std::printf("[FSM] Transit mismatch: this(%p) != aCurState(%p)\n", this, aCurState);
			return nullptr;
		}

		const TStateID aStateID = __mTransitionTable[ToIndex(pTriggerID)];
		if (aStateID == TStateID::Invalid)
		{
			std::printf("[FSM] No transition defined: stateId=%d, trigger=%d\n", ToIndex(__mStateID), ToIndex(pTriggerID));			
			return nullptr;
		}

		if (aStateID == __mStateID)
		{
			std::printf("[FSM][Transit] <State=%d> trigger=%d -> TargetState == CurrentState. No transition.\n", ToIndex(__mStateID), ToIndex(pTriggerID));
			return nullptr;
		}

		if (!IsTransitable(pChar, pTriggerID))
		{
			std::printf("[FSM] IsTransitable() = false. stateId=%d, trigger=%d\n", ToIndex(__mStateID), ToIndex(pTriggerID));
			return nullptr;
		}

		aNextState = __mFsm->GetState(aStateID);
		if (aNextState == nullptr)
		{
			std::printf("[FSM] GetState(%d) returned nullptr. Transition aborted.\n", ToIndex(aStateID));
			return nullptr;
		}

		pChar->SetState(aNextState);
	}

	if (aNextState != nullptr)
	{
		aNextState->OnEnter(pChar);
	}

	return aNextState;
}

DWORD CFsmState::PostStateUpdate(DWORD pAfter, CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());
	assert(nullptr != pChar);

	OnTaskFsmUpdate aPayload;
	aPayload.mChar = pChar;
	aPayload.mSeq = pChar->GetStateSeq();
	aPayload.mStateID = ToIndex(__mStateID);
	aPayload.mVal = pVal;

	DWORD aRv = 0;
	//aRv = CTaskScheduler::This().RegisterTask(pAfter, static_cast<const VOID*>(&aPayload), sizeof(OnTaskFsmUpdate));
	if (0 < aRv)
	{
		std::printf("[FSM] PostStateUpdate: RegisterTask failed (code=%lu, state=%d)\n", aRv, ToIndex(__mStateID));
	}

	return aRv;
}

void CIdle::OnEnter(CChar* pChar) const
{
	assert(IsOpen());

	PostStateUpdate(IDLE_TICK_INTERVAL_PASSIVE, pChar);
}

void CIdle::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());

	double aChance = (double)std::rand() / RAND_MAX;
	if (aChance < 0.30) // 30% È®·ü
	{
		pChar->Walk();
	}
	else
	{
		PostStateUpdate(IDLE_TICK_INTERVAL_PASSIVE, pChar);
	}	
}

void CIdle::_BuildTransitionTable()
{
	_RegisterTransitions(
		{
			{TTriggerID::Stroll, TStateID::Stroll},
			{TTriggerID::Aggro, TStateID::Combat},
			{TTriggerID::Die, TStateID::Dead}
		});	
}

void CStroll::OnEnter(CChar* pChar) const
{
	assert(IsOpen());

	PostStateUpdate(MOVE_TICK_INTERVAL, pChar);
}

void CStroll::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());

	PostStateUpdate(MOVE_TICK_INTERVAL, pChar);
}

void CStroll::_BuildTransitionTable()
{
	_RegisterTransitions(
		{
			{TTriggerID::Disappear, TStateID::Idle},
			{TTriggerID::Aggro, TStateID::Combat},
			{TTriggerID::Die, TStateID::Dead}
		});
}

void CCombat::OnEnter(CChar* pChar) const
{
	assert(IsOpen());	
}

void CCombat::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());	
}

void CCombat::_BuildTransitionTable()
{
	_RegisterTransitions(
		{
			{TTriggerID::Disappear, TStateID::Idle},
			{TTriggerID::Die, TStateID::Dead}
		});
}

void CDead::OnEnter(CChar* pChar) const
{
	assert(IsOpen());

	PostStateUpdate(DIE_TICK_INTERVAL, pChar);
}

void CDead::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());

	pChar->Ghost();
}

bool CDead::IsTransitable(CChar* pChar, TTriggerID pTriggerID) const
{
	assert(pChar != nullptr);	
	assert((0 <= ToIndex(pTriggerID)) && (ToIndex(pTriggerID) < ToIndex(TTriggerID::Max)));

	bool aIsLive = pChar->IsAlive();
	
	if (pTriggerID == TTriggerID::Respawn && aIsLive)
		return true;

	if (pTriggerID == TTriggerID::Ghost && !aIsLive)
		return true;

	return false;
}

void CDead::_BuildTransitionTable()
{
	_RegisterTransitions(
		{
			{TTriggerID::Respawn, TStateID::Idle},
			{TTriggerID::Ghost, TStateID::Ghost}
		});
}

void CGhost::OnEnter(CChar* pChar) const
{
	assert(IsOpen());	
}

void CGhost::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());

	pChar->Respawn();
}

bool CGhost::IsTransitable(CChar* pChar, TTriggerID pTriggerID) const
{
	assert(pChar != nullptr);
	assert((0 <= ToIndex(pTriggerID)) && (ToIndex(pTriggerID) < ToIndex(TTriggerID::Max)));

	bool aIsLive = pChar->IsAlive();

	if (pTriggerID == TTriggerID::Respawn && aIsLive)
		return true;

	return false;
}

void CGhost::_BuildTransitionTable()
{
	_RegisterTransitions(
		{
			{TTriggerID::Respawn, TStateID::Idle}
		});
}

void CActiveIdle::OnEnter(CChar* pChar) const
{
	assert(IsOpen());

	PostStateUpdate(IDLE_TICK_INTERVAL_ACTIVE, pChar);
}

void CActiveIdle::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());

	PostStateUpdate(IDLE_TICK_INTERVAL_ACTIVE, pChar);
}

void CActiveStroll::OnEnter(CChar* pChar) const
{
	assert(IsOpen());
}

void CActiveStroll::OnUpdate(CChar* pChar, DWORD pVal) const
{
	assert(IsOpen());	

	CStroll::OnUpdate(pChar, pVal);
}