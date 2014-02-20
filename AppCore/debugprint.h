// Defines debugging output macro.
#pragma once

#include <sstream>
#include <iostream>

#ifdef _WIN32
// On windows, must include this header to get OutputDebugString.
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Define debug break macro.
#ifdef _MSC_VER
    #ifdef _X86_
        #define DBGBRK { __asm{int 3}; }
    #else
        #include <intrin.h>
        #define DBGBRK  { __debugbreak(); }
    #endif
#else
    #define DBGBRK { assert(false); }
#endif

#ifdef _WIN32
// On windows, print to std::cout and OutputDebugString.
#define DBPRINT(s)                              \
    {                                           \
        std::ostringstream _oss_;               \
        _oss_ << s;                             \
        OutputDebugStringA(_oss_.str().c_str()); \
        std::cout << _oss_.str();               \
    }
#else
// On all other systems, simply print std::cout.
#define DBPRINT(s)                              \
    {                                           \
        std::cout << s;                         \
    }
#endif

#ifdef _WIN32
// On windows, print to std::cout and OutputDebugString.
#define DBPRINTMAT(m)                                   \
    {                                                   \
        OutputDebugStringA("[");                         \
        std::cout << "[";                               \
        for (int _r_ = 0; _r_ < m.rows(); _r_++)        \
        {                                               \
            std::ostringstream _oss_;                   \
            _oss_.precision(15);                        \
            for (int _c_ = 0; _c_ < m.cols(); _c_++)    \
            {                                           \
                _oss_ << m(_r_,_c_);                    \
                if (_c_ != m.cols()-1)                  \
                    _oss_ << ", ";                      \
            }                                           \
            if (_r_ != m.rows()-1)                      \
                _oss_ << "; ";                          \
            OutputDebugStringA(_oss_.str().c_str());     \
            std::cout << _oss_.str();                   \
        }                                               \
        OutputDebugStringA("]\n");                       \
        std::cout << "]" << std::endl;                  \
    }
#else
// On all other systems, simply print std::cout.
#define DBPRINTMAT(m)                                   \
    {                                                   \
        std::cout << "[";                               \
        for (int _r_ = 0; _r_ < m.rows(); _r_++)        \
        {                                               \
            for (int _c_ = 0; _c_ < m.cols(); _c_++)    \
            {                                           \
                std::cout << m(_r_,_c_);                \
                if (_c_ != m.cols()-1)                  \
                    std::cout << ", ";                  \
            }                                           \
            if (_r_ != m.rows()-1)                      \
                std::cout << "; ";                      \
        }                                               \
        std::cout << "]" << std::endl;                  \
    }
#endif

// Convenience macro for inserting file and line number.
#define INSFILELINE "[" << __FILE__ << ":" << __LINE__ << "]"

// Convenience macro for printing a line.
#define DBPRINTLN(s) DBPRINT(s << std::endl)

// Convenience macro for print a line annotated with file and line.
#define DBFILELINE(s) DBPRINTLN(INSFILELINE << s)

// Convenience macro for printing a warning.
#define DBWARNING(s) DBPRINTLN("WARNING" << INSFILELINE << ": " << s)

// Convenience macro for printing an error, forces debug break.
#define DBERROR(s) { DBPRINTLN("ERROR" << INSFILELINE << ": " << s); DBGBRK; }

// Macro to initialize timing functions.
#ifdef _WIN32
// On Windows, use Windows timing.
#define DBINITTIME                                                          \
    unsigned __int64 _timerStart_;                                          \
    unsigned __int64 _timerEnd_;                                            \
    unsigned __int64 _pf_;                                                  \
    double _timeSecs_;                                                      \
    double _timerFrequency_;                                                \
    QueryPerformanceFrequency((LARGE_INTEGER*)&_pf_);                       \
    _timerFrequency_ = 1.0/(double)_pf_;
#define DBSTARTTIME                                                         \
    QueryPerformanceCounter((LARGE_INTEGER*)&_timerStart_);
#define DBPRINTTIME(s)                                                      \
    QueryPerformanceCounter((LARGE_INTEGER*)&_timerEnd_);                   \
    _timeSecs_ = ((double)(_timerEnd_ - _timerStart_))*_timerFrequency_;    \
    DBPRINTLN(s << ": " << _timeSecs_)
#else
#define DBINITTIME      DBPRINTLN("Timing only supported on Windows for now.");
#define DBSTARTTIME     DBPRINTLN("Timing only supported on Windows for now.");
#define DBPRINTTIME(s)  DBPRINTLN(s << ": timing only supported on Windows for now.");
#endif
