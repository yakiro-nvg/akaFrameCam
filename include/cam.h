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
#define CAS_GLOBAL      0
#define CAS_LOCAL_STACK 1
#define CAS_BORROWED    2
#define CAS_ID          3

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
        } _v; // CAS_GLOBAL or CAS_LOCAL_STACK
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
        } _b; // CAS_BORROWED
        struct {
#if SX_CPU_ENDIAN_BIG
                u32 _           : 3;
                u32 _index      : 21;
                u32 _generation : 8;
#else
                u32 _generation : 8;
                u32 _index      : 21;
                u32 _           : 3;
#endif
        } _i; // CAS_ID
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
                                                      , void                   *ktx
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
                                                       );

        void                  (*execute              ) (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                       );
} cam_program_t;

CAM_API
/// Initializes a new CAM instance.
struct cam_s*                   cam_new                (cam_error_t            *out_ec
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
cam_error_t                     cam_load_chunk         (struct cam_s           *cam
                                                      , const void             *buff
                                                      , int                     buff_size
                                                       );

CAM_API
/// Makes an address that pointed to given `buff`.
cam_error_t                     cam_address_make       (struct cam_s           *cam
                                                      , void                   *buff
                                                      , bool                    borrow
                                                      , cam_address_t          *out_address
                                                       );

CAM_API
/// Drops (not deallocate, since it's just a handle.
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
                                                      , void                   *ktx
                                                       );

CAM_API
/// Calls a program with a continuation `k`.
void                            cam_call               (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

CAM_API
/// Goes back to the calling program.
void                            cam_go_back            (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Creates a logical (green) task, cooperative scheduled with a finalizer `k`.
cam_tid_t                       cam_task_new           (struct cam_s           *cam
                                                      , cam_error_t            *out_ec
                                                      , cam_pid_t               entry
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

CAM_API
/// Releases allocated resources and deletes the task.
void                            cam_task_delete        (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Returns top program (last activation record).
cam_pid_t                       cam_top_program        (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Yields with a continuation `k`.
void                            cam_yield              (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

CAM_API
/// Starts or resumes, returns when the task suspends or finishes its execution.
void                            cam_resume             (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

CAM_API
/// Gets provider (at given `index`) state on local stack.
void*                           cam_get_tlpvs          (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     index
                                                       );

CAM_API
/// Sets provider (at given `index`) state on local stack.
void                            cam_set_tlpvs          (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     index
                                                      , void                   *state
                                                       );

CAM_API
/// Resizes the local stack so it contains more `bytes`, returns previous top.
u8*                             cam_push               (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

CAM_API
/// Resizes the local stack so it contains less `bytes`, returns next top.
u8*                             cam_pop                (struct cam_s           *cam
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
bool                            cam_is_alive_task      (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_CAM_H_
