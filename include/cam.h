/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_H_
#define _CAM_H_

#include <cam/prereq.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cam_s;

/// Address space.
#define CAS_GLOBAL       0
#define CAS_THREAD_STACK 1
#define CAS_BORROWED     2
#define CAS_PROGRAM_ID   3

#pragma pack(push, 1)
typedef union cam_address_u {
        struct {
#if SX_CPU_ENDIAN_BIG
                u32 _       : 1;
                u32 _space  : 2;
                u32 _offset : 29;
#else
                u32 _offset : 29;
                u32 _space  : 2;
                u32 _       : 1;
#endif
        } _v;
        struct {
#if SX_CPU_ENDIAN_BIG
                u32 _       : 3;
                u32 _index  : 4;
                u32 _offset : 25;
#else
                u32 _offset : 25;
                u32 _index  : 4;
                u32 _       : 3;
#endif
        } _b;
        u32 _u;
} cam_address_t;

typedef struct cam_pid_s {
        u32 _u;
} cam_pid_t;

typedef struct cam_tid_s {
        u32 _u;
} cam_tid_t;
#pragma pack(pop)

typedef void                  (*cam_k_t              ) (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

typedef struct cam_provider_s {
        int                     index;

        void                  (*t_entry              ) (struct cam_provider_s  *provider
                                                      , cam_tid_t               tid
                                                       );

        void                  (*t_leave              ) (struct cam_provider_s  *provider
                                                      , cam_tid_t               tid
                                                       );

        cam_pid_t             (*resolve              ) (struct cam_provider_s  *provider
                                                      , const char             *name
                                                       );
} cam_provider_t;

typedef struct cam_program_s {
        cam_provider_t         *provider;

        void                  (*load                 ) (struct cam_s           *cam
                                                      , cam_pid_t               pid
                                                       );

        void                  (*prepare              ) (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                       );

        void                  (*execute              ) (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                       );
} cam_program_t;

CAM_API
/// Initializes a new CAM instance.
struct cam_s*                   cam_new                (int                    *out_ec
                                                       );

CAM_API
/// Releases allocated resources and deletes the instance.
void                            cam_delete             (struct cam_s           *cam
                                                       );

CAM_API
/// Registers a new program provider.
void                            cam_add_provider       (struct cam_s           *cam
                                                      , cam_provider_t         *provider
                                                       );

CAM_API
/// Registers a new program so it will be managed by CAM's built-in provider.
void                            cam_add_program        (struct cam_s           *cam
                                                      , const char             *name
                                                      , cam_program_t          *program
                                                       );

CAM_API
/// Adds a new code chunk, `buff` will not be copied internally, dont' free it.
int                             cam_load_chunk         (struct cam_s           *cam
                                                      , const void             *buff
                                                      , int                     buff_size
                                                       );

CAM_API
/// Borrows an external buffer and returns managed address.
int                             cam_address_borrow     (struct cam_s           *cam
                                                      , void                   *buff
                                                      , cam_address_t          *out_address
                                                       );

CAM_API
/// Relocates an address, points to a new location.
void                            cam_address_relocate   (struct cam_s           *cam
                                                      , cam_address_t           address
                                                      , void                   *buff
                                                       );

CAM_API
/// Drops (not deallocate, since it's borrowed) `address`.
void                            cam_address_drop       (struct cam_s           *cam
                                                      , cam_address_t           address
                                                       );

CAM_API
/// Resolves the buffer that is pointed to by given `address`.
void*                           cam_address_buffer     (struct cam_s           *cam
                                                      , cam_address_t           address
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Searches for a `name`d program.
cam_pid_t                       cam_resolve            (struct cam_s           *cam
                                                      , const char             *name
                                                       );

CAM_API
/// A no-op continuation.
void                            cam_nop_k              (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Calls a program with a continuation `k`.
void                            cam_call               (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                       );

CAM_API
/// Creates a logical (green) thread, cooperative scheduled.
cam_tid_t                       cam_thread_new         (struct cam_s           *cam
                                                      , int                    *out_ec
                                                      , cam_pid_t               entry
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                       );

CAM_API
/// Releases allocated resources and deletes the thread.
void                            cam_thread_delete      (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Yields `thread` with a continuation `k`.
void                            cam_thread_yield       (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_k_t                 k
                                                       );

CAM_API
/// Starts or resumes `thread`, returns when the thread suspends or finishes its execution.
void                            cam_thread_resume      (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Gets thread local provider state at given `index`.
void*                           cam_thread_get_tlpvs   (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     index
                                                       );

CAM_API
/// Gets thread local provider state at given `index`.
void                            cam_thread_set_tlpvs   (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     index
                                                      , void                   *state
                                                       );

CAM_API
/// Resizes the thread stack so it contains more `bytes`, returns previous top.
u8*                             cam_thread_push        (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

CAM_API
/// Resizes the thread stack so it contains less `bytes`, returns next top.
u8*                             cam_thread_pop         (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     bytes
                                                       );

CAM_API
/// Allocates global memory, which is grow-only (can't be deallocated).
u8*                             cam_global_grow        (struct cam_s           *cam
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

CAM_API
/// Checks whether given id is alive.
bool                            cam_is_alive_program   (struct cam_s           *cam
                                                      , cam_pid_t               pid
                                                       );

CAM_API
/// Checks whether given id is alive.
bool                            cam_is_alive_thread    (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_CAM_H_
