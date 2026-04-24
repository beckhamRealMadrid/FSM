#include "pch.h"
#include "CChar.h"
#include "CFsmMgr.h"

CChar::CChar()
{
	__mLocker.Open(FALSE);

	__ResetAttr();
}

CChar::~CChar()
{
	__Dtor();

	__mLocker.Close();
}

VOID CChar::__Dtor(VOID)
{
	
}

VOID CChar::__ResetAttr(VOID)
{
	_mState = nullptr;
	_mStateSeq = 0;
	_mStateCallCnt = 1;	
	_mHp = 0;
}

VOID CChar::Lock() const
{
	__mLocker.Lock();
}

VOID CChar::Unlock() const
{
	__mLocker.Unlock();
}

BOOL CChar::IsLock() const
{
	return __mLocker.IsLock();
}

void CChar::SetState(const CFsmState* pState)
{
	assert(nullptr != pState);

	_mState = pState;

	++_mStateSeq;
	_mStateCallCnt = 1;
}

const CFsmState* CChar::GetState() const
{
	return _mState;
}

DWORD CChar::GetStateSeq() const
{
	return _mStateSeq;
}

DWORD CChar::GetStateCallCnt() const
{
	return _mStateCallCnt;
}

bool CChar::IsAlive() const
{
	return 0 < _mHp;
}

bool CChar::Spawn(TAiType pAiType)
{
	const CFsm* aFsm = CFsmMgr::This().GetFsm(pAiType);
	if (aFsm == nullptr)
	{
		return false;
	}

	_mState = aFsm->GetState(TStateID::Idle);
	_mHp = 100;

	_mState->OnEnter(this);

	return true;
}

DWORD CChar::Walk()
{
	_mState->Transit(TTriggerID::Stroll, this);	

	return 0;
}

void CChar::Idle()
{
	_mState->Transit(TTriggerID::Disappear, this);	
}

void CChar::Hit(CChar* pAttacker, INT pDamage)
{
	{
		TLocker lock(this);
		_mHp = max(0, _mHp - pDamage);
	}

	if (_mHp <= 0)
	{
		Die();
		return;
	}

	if (!_mState->IsBattle())
	{
		_mState->Transit(TTriggerID::Aggro, this);
	}	
}

void CChar::Die()
{
	if (_mState && _mState->GetStateID() == TStateID::Dead)
		return;

	{
		TLocker	aLocker(this);
		_mHp = 0;
	}

	_mState->Transit(TTriggerID::Die, this);	
}

void CChar::Ghost()
{
	_mState->Transit(TTriggerID::Ghost, this);	
}

void CChar::Respawn()
{
	{
		TLocker	aLocker(this);
		_mHp = 100;
	}

	_mState->Transit(TTriggerID::Respawn, this);	
}

// 시퀀스/타입이 맞으면 현재 상태의 OnUpdate 호출
// - 상태 시퀀스와 상태 ID가 현재와 일치하는 경우에만 OnUpdate 실행
// - 상태가 이미 바뀌었거나(ID/시퀀스 불일치) 지연된 요청이면 무시한다
void CChar::UpdateState(DWORD pSeq, INT pStateID, DWORD pVal)
{
	assert(!IsLock());

	const CFsmState* aState = _mState;
	bool aIs = false;
	{
		TLocker	aLocker(this);
		if ((_mStateSeq == pSeq) && (pStateID == ToIndex(aState->GetStateID())))
		{
			++_mStateCallCnt;
			aIs = true;
		}
	}

	if (aIs)
	{
		aState->OnUpdate(this, pVal);
	}
}