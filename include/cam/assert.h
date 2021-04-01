/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_ASSERT_H_
#define _CAM_ASSERT_H_

#ifdef CAM_ASSERTION
#include <assert.h>
#ifndef CAM_ASSERT
#define CAM_ASSERT assert
#endif
#else
#define CAM_ASSERT
#endif

#endif // !_CAM_ASSERT_H_
