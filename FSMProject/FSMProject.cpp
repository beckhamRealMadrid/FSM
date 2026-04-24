// FSMProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "CFsmMgr.h"
#include "CChar.h"
#include <iostream>

int main()
{
	CFsmMgr& aFsmMgr = CFsmMgr::This();
	if (aFsmMgr.Open() != 0)
	{
		std::cout << "FSM 초기화 실패!" << std::endl;
		return -1;
	}

	CChar aChar;	
	if (!aChar.Spawn(TAiType::Passive))
	{
		std::cout << "캐릭터 Spawn 실패!" << std::endl;
		return -1;
	}

	aFsmMgr.Close();
}