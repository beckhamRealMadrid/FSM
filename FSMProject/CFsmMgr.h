#pragma once

#include "CFsm.h"

class CFsmMgr
{
public:
	static CFsmMgr&	This();
private:
	using TFsmPtr = std::unique_ptr<CFsm>;
	using TFsmArray = std::array<TFsmPtr, ToIndex(TAiType::Max)>;
private:
	static CFsmMgr	__mSingleton;
public:
	DWORD Open();
	void Close();
	bool IsOpen() const;
	const CFsm*	GetFsm(TAiType pAiType) const;
private:
	template <typename TFsm>
	void _RegisterFsm()
	{
		auto aPtr = std::make_unique<TFsm>();
		auto aIdx = ToIndex(aPtr->GetAiType());
		
		__mFsms[aIdx] = std::move(aPtr);
	}
	void __SetFsms();
private:
	bool __mIsOpen;
	TFsmArray __mFsms;
};
