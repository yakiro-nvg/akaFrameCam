/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_ERRNO_H_
#define _CAM_ERRNO_H_

/// Error code.
#define CEC_SUCCESS       0
#define CEC_UNEXPECTED    1
#define CEC_NOT_FOUND     2
#define CEC_NOT_PROGRAM   3
#define CEC_BAD_CHUNK     4
#define CEC_NOT_SUPPORTED 5
#define CEC_NO_MEMORY     6
typedef int cam_error_t;

#endif // !_CAM_ERRNO_H_
