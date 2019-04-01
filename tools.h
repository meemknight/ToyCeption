/////////////////////////////////////////////
//tools.h
//Copyright(c) 2019 Luta Vlad
//https://github.com/meemknight/OpenGLEngine
/////////////////////////////////////////////

///define FORCE_LOG
///to controll the behavour of the log or just 
///change from debug to releasee

#pragma once
#include <iostream>
#include <Windows.h>

#define FORCE_LOG

#ifdef _DEBUG
#define FORCE_LOG
#endif

#ifdef FORCE_LOG
#define MAIN main()
#else
#define MAIN WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
#endif

#ifdef FORCE_LOG
inline void llog()
{
	std::cout << "\n";
}

template<class F, class ...T>
inline void llog(F f, T ...args)
{
	std::cout << f << " ";
	llog(args...);
}
#else
template<class F, class ...T>
inline void llog(F f, T ...args)
{

}
#endif

///warning log
#ifdef FORCE_LOG
inline void wlog()
{
	std::cout << "\n";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}

template<class F, class ...T>
inline void wlog(F f, T ...args)
{	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
	std::cout << f << " ";
	wlog(args...);
}
#else
template<class F, class ...T>
inline void wlog(F f, T ...args)
{

}
#endif

///important log
#ifdef FORCE_LOG
inline void ilog()
{
	std::cout << "\n";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}

template<class F, class ...T>
inline void ilog(F f, T ...args)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
	std::cout << f << " ";
	ilog(args...);
}
#else
template<class F, class ...T>
inline void ilog(F f, T ...args)
{

}
#endif

///good log
#ifdef FORCE_LOG
inline void glog()
{
	std::cout << "\n";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}

template<class F, class ...T>
inline void glog(F f, T ...args)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
	std::cout << f << " ";
	glog(args...);
}
#else
template<class F, class ...T>
inline void glog(F f, T ...args)
{

}
#endif

///error log
#ifdef FORCE_LOG
inline void elog()
{
	std::cout << "\n";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
}

template<class F, class ...T>
inline void elog(F f, T ...args)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	std::cout << f << " ";
	elog(args...);
}
#else
template<class F, class ...T>
inline void elog(F f, T ...args)
{

}
#endif
