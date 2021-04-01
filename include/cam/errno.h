/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_ERRNO_H_
#define _CAM_ERRNO_H_

/// Error code.
typedef enum {
        CEC_SUCCESS,
        CEC_UNEXPECTED,
        CEC_NOT_FOUND,
        CEC_NOT_PROGRAM,
        CEC_BAD_CHUNK,
        CEC_BAD_ARGUMENTS,
        CEC_NO_MEMORY
} cam_error_t;

#endif // !_CAM_ERRNO_H_
