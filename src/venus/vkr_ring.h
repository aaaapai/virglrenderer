/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VKR_RING_H
#define VKR_RING_H

#include "vkr_common.h"

struct virgl_context;

struct memory_region {
   size_t offset;
   size_t size;
};

/* the layout of a ring in a virgl_resource */
struct vkr_ring_layout {
   struct memory_region head;
   struct memory_region tail;
   struct memory_region status;
   struct memory_region buffer;
   struct memory_region extra;
};

static_assert(ATOMIC_INT_LOCK_FREE == 2 && sizeof(atomic_uint) == 4,
              "vkr_ring_shared requires lock-free 32-bit atomic_uint");

/* pointers to a ring in a virgl_resource */
struct vkr_ring_shared {
   volatile atomic_uint *head;
   const volatile atomic_uint *tail;
   volatile atomic_uint *status;
   const void *buffer;

   void *extra;
};

struct vkr_ring {
   /* used by the caller */
   vkr_object_id id;
   struct list_head head;

   struct vkr_ring_shared shared;
   uint32_t buffer_size;
   uint32_t buffer_mask;
   uint32_t cur;
   void *cmd;

   size_t extra_size;

   struct virgl_context *context;
   uint64_t idle_timeout;

   mtx_t mutex;
   cnd_t cond;
   thrd_t thread;
   atomic_bool started;
   atomic_bool pending_notify;
};

struct vkr_ring *
vkr_ring_create(const struct vkr_ring_layout *layout,
                void *shared,
                struct virgl_context *ctx,
                uint64_t idle_timeout);

void
vkr_ring_destroy(struct vkr_ring *ring);

void
vkr_ring_start(struct vkr_ring *ring);

bool
vkr_ring_stop(struct vkr_ring *ring);

void
vkr_ring_notify(struct vkr_ring *ring);

bool
vkr_ring_write_extra(struct vkr_ring *ring, size_t offset, uint32_t val);

#endif /* VKR_RING_H */