#include "pch.h"
#include "CFsmMgr.h"

CFsmMgr	CFsmMgr::__mSingleton;
CFsmMgr& CFsmMgr::This()
{
	return __mSingleton;
}

bool CFsmMgr::IsOpen() const
{
	return __mIsOpen;
}

const CFsm* CFsmMgr::GetFsm(TAiType pAiType) const
{
	assert(IsOpen());
	
	return __mFsms[ToIndex(pAiType)].get();
}

void CFsmMgr::__SetFsms()
{
	_RegisterFsm<CPassive>();
	_RegisterFsm<CActive>();
}

DWORD CFsmMgr::Open()
{
	assert(!IsOpen());

	DWORD aRv = 0;

	__SetFsms();

	for (auto& aFsm : __mFsms)
	{
		if (!aFsm)
			continue;

		DWORD aRv = aFsm->Open(aFsm->GetStateCnt());
		if (0 < aRv)
		{
			return aRv;
		}
	}

	__mIsOpen = true;

	return 0;
}

void CFsmMgr::Close()
{
	if (!IsOpen())
		return;

	for (auto& aFsm : __mFsms)
	{
		if (aFsm)
			aFsm->Close();
	}

	__mIsOpen = false;
}