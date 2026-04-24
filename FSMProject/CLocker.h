#pragma once

constexpr DWORD DEFAULT_SPIN_COUNT = 4000;
constexpr DWORD MANY_RACE_FLAG = 0x80000000;

class CLocker
{
public:
	CLocker();
	~CLocker();	
public:
	DWORD	Open(BOOL pIsManyRace, DWORD pSpin = DEFAULT_SPIN_COUNT);
	VOID	Close();
	bool	IsOpen() const;
	VOID	Lock() const;
	VOID	Unlock() const;
	bool	LockTry() const;
	BOOL	IsLock() const;
private:
	VOID	__Dtor();
	VOID	__ResetAttr();
	DWORD	__InitializeCriticalSectionWithSpin(BOOL pIsManyRace, DWORD pSpin);
private:
	bool	__mIsOpen;
	mutable CRITICAL_SECTION	__mSec;
};

// RAII(Resource Acquisition Is Initialization)
// 가능하다면 반드시 CLockerAuto를 사용한다. __TLocker는 Lock()/Unlock()가 있어야 한다.
template <typename __TLocker>
class CLockerAuto
{
public:
	typedef	__TLocker	TLocker;
public:
	explicit CLockerAuto(const __TLocker& pLocker) : __mLocker(&pLocker) { __mLocker->Lock(); }
	explicit CLockerAuto(const __TLocker* pLocker) : __mLocker(pLocker) { assert(nullptr != pLocker); __mLocker->Lock(); }
	~CLockerAuto() { if (__mLocker) __mLocker->Unlock(); }
private:
	CLockerAuto(const CLockerAuto&) = delete;
	CLockerAuto& operator=(const CLockerAuto&) = delete;
private:
	const __TLocker*	__mLocker = nullptr;
};

// LockTry같은 경우엔 CLockerAuto를 사용할 수 없으므로 LockTry가 성공한 경우 Unlock만 자동으로 한다. __TLocker는 Unlock()가 있어야 한다.
template <typename __TLocker>
class CUnlockerAuto
{
public:
	typedef	__TLocker	TLocker;
public:
	explicit CUnlockerAuto(const __TLocker& pLocker) : __mLocker(&pLocker) {}
	explicit CUnlockerAuto(const __TLocker* pLocker) : __mLocker(pLocker) { assert(nullptr != pLocker); }
	~CUnlockerAuto() { if (__mLocker)__mLocker->Unlock(); }
private:
	CUnlockerAuto(const CUnlockerAuto&) = delete;
	CUnlockerAuto& operator=(const CUnlockerAuto&) = delete;
private:
	const __TLocker*	__mLocker = nullptr;
};