#pragma once
#ifndef DLL_H
#define DLL_H

#ifdef DLL_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

#endif