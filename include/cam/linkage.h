/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_LINKAGE_H_
#define _CAM_LINKAGE_H_

#if SX_PLATFORM_WINDOWS
#ifdef CAM_EXPORTS
#define CAM_API __declspec(dllexport)
#else
#define CAM_API __declspec(dllimport)
#endif
#else
#define CAM_API
#endif

#endif // !_CAM_LINKAGE_H_
