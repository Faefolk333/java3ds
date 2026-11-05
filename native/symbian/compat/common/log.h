#pragma once
// Small standalone CPU-probe adapter. Original EKA2L1 diagnostic templates are
// retained verbatim; register/fault values are reported by the platform bridge.
#include <cstdio>
#include <cstdint>
namespace eka2l1 { enum { CPU_DYNCOM }; }
#define LOG_TRACE(...) do {} while(0)
#define LOG_DEBUG(...) do {} while(0)
#define LOG_INFO(...) do {} while(0)
#define LOG_WARN(category,message,...) do { std::fprintf(stderr,"SYMBIAN_CPU warning: %s\n",message); } while(0)
#define LOG_ERROR(category,message,...) do { std::fprintf(stderr,"SYMBIAN_CPU error: %s\n",message); } while(0)
#define LOG_CRITICAL(category,message,...) LOG_ERROR(category,message)
