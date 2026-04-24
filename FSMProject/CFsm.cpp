#include "pch.h"
#include "CFsm.h"

CFsm::CFsm(TAiType pAiType) : __mAiType(pAiType)
{
	__ResetAttr();
}

CFsm::~CFsm()
{
	__Dtor();
}

void CFsm::__ResetAttr()
{
	
}

void CFsm::__Dtor()
{
	
}

DWORD CFsm::Open(DWORD pStateCnt)
{
	__mStates.clear();
	__mStates.resize(pStateCnt);

	__SetStates();

	for (auto& aState : __mStates)
	{
		if (aState == nullptr)
		{
			return ERROR_INVALID_STATE;
		}
		
		DWORD aRv = aState->Open(aState->GetTriggerCnt(), this);
		if (0 < aRv)
		{
			return aRv;
		}
	}

	return 0;
}

void CFsm::Close()
{
	__Dtor();

	if (!__mStates.empty())
	{
		for (auto& aState : __mStates)
		{
			if (aState)
				aState->Close();
		}
	}

	__mStates.clear();
	__ResetAttr();
}

bool CFsm::IsOpen() const
{
	return !__mStates.empty();
}

TAiType	CFsm::GetAiType() const
{
	return __mAiType;
}

DWORD CFsm::GetStateCnt() const
{
	return ToIndex(TStateID::Max);
}

const CFsmState* CFsm::GetState(TStateID pStateID) const
{
	assert(IsOpen());
	assert((0 <= ToIndex(pStateID)) && (ToIndex(pStateID) < __mStates.size()));

	return __mStates[ToIndex(pStateID)].get();
}

void CPassive::__SetStates()
{
	_RegisterState<CIdle>();
	_RegisterState<CStroll>();
	_RegisterState<CCombat>();
	_RegisterState<CDead>();
	_RegisterState<CGhost>();
}

void CActive::__SetStates()
{
	_RegisterState<CActiveIdle>();
	_RegisterState<CActiveStroll>();
	_RegisterState<CCombat>();
	_RegisterState<CDead>();
	_RegisterState<CGhost>();
}