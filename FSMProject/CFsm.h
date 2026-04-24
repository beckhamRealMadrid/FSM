#pragma once

#include "CFsmState.h"

namespace Fsm
{
	enum class EAiType : int
	{
		Passive = 1,	// 비선공형(Idle, Stroll 중심 행동)
		Active,			// 선공형(Idle 상태에서도 적탐색 및 선공가능)
		Max
	};
}

using TAiType = Fsm::EAiType;

class CChar;

class CFsm
{
private:
	using TStatePtr = std::unique_ptr<CFsmState>;
	using TStateList = std::vector<TStatePtr>;
public:
	explicit CFsm(TAiType pAiType);
	virtual ~CFsm();
	CFsm(const CFsm&) = delete;
	CFsm& operator=(const CFsm&) = delete;
public:
	DWORD Open(DWORD pStateCnt);
	void Close();
	bool IsOpen() const;
	TAiType GetAiType() const;
	const CFsmState* GetState(TStateID pStateID) const;
	virtual DWORD GetStateCnt() const;
protected:
	template <typename TState>
	void _RegisterState()
	{
		auto aPtr = std::make_unique<TState>();
		auto aId = ToIndex(aPtr->GetStateID());
		
		__mStates[aId] = std::move(aPtr);
	}	
private:
	virtual	void __SetStates() = 0;
	void __Dtor();
	void __ResetAttr();
private:
	TAiType __mAiType;
	TStateList __mStates;
};

class CPassive : public CFsm
{
public:
	CPassive() : CFsm(TAiType::Passive) {}
private:
	void __SetStates() override;
};

class CActive : public CFsm
{
public:
	CActive() : CFsm(TAiType::Active) {}
private:
	void __SetStates() override;
};