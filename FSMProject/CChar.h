#pragma once

#include "CLocker.h"
#include "CFsm.h"

class CFsmState;

class CChar
{
public:
	using TLocker = CLockerAuto<CChar>;
public:		
	CChar();
	virtual				~CChar();
	VOID				Lock() const;
	VOID				Unlock() const;
	BOOL				IsLock() const;
	void				SetState(const CFsmState* pState);
	const CFsmState*	GetState() const;
	DWORD				GetStateSeq() const;
	DWORD				GetStateCallCnt() const;
	bool				IsAlive() const;
	bool				Spawn(TAiType pAiType);
	DWORD				Walk();
	void				Idle();
	void				Hit(CChar* pAttacker, INT pDamage);
	void				Die();
	void				Ghost();
	void				Respawn();
	void				UpdateState(DWORD pSeq, INT pStateID, DWORD pVal);
private:
	VOID				__Dtor(VOID);
	VOID				__ResetAttr(VOID);
protected:
	DWORD				_mStateSeq;		// 상태가 바뀔 때마다 증가하는 번호 (동일 상태 실행 여부 확인용)
	DWORD				_mStateCallCnt;	// 같은 상태에서 CFsmState::OnUpdate 호출된 횟수
	const CFsmState*	_mState;
	INT					_mHp;
private:
	CLocker				__mLocker;
};
