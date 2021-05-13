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
#define CAS_ID          2

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

typedef struct cam_args_s {
        int  arity;
        u32 *at;
} cam_args_t;

typedef cam_address_t         (*cam_on_unresolved_t  ) (struct cam_s           *cam
                                                      , const char             *name
                                                       );

typedef void                  (*cam_k_t              ) (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , void                   *ktx
                                                       );

typedef struct cam_provider_s {
        int                     index;
        char                    name[4];
        void                   *userdata;

        void                  (*fiber_entry          ) (struct cam_provider_s  *provider
                                                      , cam_address_t           fid
                                                       );

        void                  (*fiber_leave          ) (struct cam_provider_s  *provider
                                                      , cam_address_t           fid
                                                       );

        cam_address_t         (*resolve              ) (struct cam_provider_s  *provider
                                                      , const char             *name
                                                       );
} cam_provider_t;

typedef struct cam_program_s {
        cam_provider_t         *provider;
        void                   *userdata;

        void                  (*execute              ) (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , cam_address_t           pid
                                                       );
} cam_program_t;

#pragma pack(pop)

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
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Searches for a `name`d program, returns NULL if not found.
cam_address_t                   cam_resolve            (struct cam_s           *cam
                                                      , const char             *name
                                                       );

CAM_API
/// Sets unresolved program callback, required to support load-on-demand.
void                            cam_on_unresolved      (struct cam_s           *cam
                                                      , cam_on_unresolved_t     callback
                                                       );

CAM_API
/// A no-op continuation.
void                            cam_nop_k              (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , void                   *ktx
                                                       );

CAM_API
/// Calls a program with a continuation `k`.
void                            cam_call               (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , cam_address_t           pid
                                                      , u32                    *args
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

CAM_API
/// Goes back to the calling program.
void                            cam_go_back            (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Creates a fiber (logical thread), cooperative scheduled with a finalizer `k`.
cam_address_t                   cam_fiber_new          (struct cam_s           *cam
                                                      , cam_error_t            *out_ec
                                                      , void                   *userdata
                                                      , cam_address_t           entry_pid
                                                      , u32                    *args
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

CAM_API
/// Releases allocated resources and deletes the fiber.
void                            cam_fiber_delete       (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Returns fiber's opaque userdata.
void*                           cam_fiber_userdata     (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Returns top program (last activation record).
cam_address_t                   cam_top_program        (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Yields with a continuation `k`.
void                            cam_yield              (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

CAM_API
/// Starts or resumes, returns when the fiber suspends or finishes its execution.
void                            cam_resume             (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Gets provider (at given `index`) state on local stack.
void*                           cam_get_tlpvs          (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , int                     index
                                                       );

CAM_API
/// Sets provider (at given `index`) state on local stack.
void                            cam_set_tlpvs          (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , int                     index
                                                      , void                   *state
                                                       );

CAM_API
/// Resizes the local stack so it contains more `bytes`, returns previous top.
u8*                             cam_push               (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

CAM_API
/// Resizes the local stack so it contains less `bytes`, returns next top.
u8*                             cam_pop                (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                      , int                     bytes
                                                       );

CAM_API
/// Returns address to the start of current function's stack frame.
cam_address_t                   cam_bp                 (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Returns address to top of the stack frame.
cam_address_t                   cam_sp                 (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Returns passed argument list.
cam_args_t                      cam_args               (struct cam_s           *cam
                                                      , cam_address_t           fid
                                                       );

CAM_API
/// Allocates global memory, which is grow-only (can't be deallocated).
u8*                             cam_global_grow        (struct cam_s           *cam
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_CAM_H_
