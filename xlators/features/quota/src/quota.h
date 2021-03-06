/*
  Copyright (c) 2008-2010 Gluster, Inc. <http://www.gluster.com>
  This file is part of GlusterFS.

  GlusterFS is free software; you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation; either version 3 of the License,
  or (at your option) any later version.

  GlusterFS is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include "xlator.h"
#include "call-stub.h"
#include "defaults.h"
#include "byte-order.h"
#include "common-utils.h"
#include "quota-mem-types.h"

#define QUOTA_XATTR_PREFIX      "trusted."
#define DIRTY                   "dirty"
#define SIZE                    "size"
#define CONTRIBUTION            "contri"
#define VAL_LENGTH              8
#define READDIR_BUF             4096

#define QUOTA_STACK_DESTROY(_frame, _this)              \
        do {                                            \
                quota_local_t *_local = NULL;           \
                _local = _frame->local;                 \
                _frame->local = NULL;                   \
                STACK_DESTROY (_frame->root);           \
                quota_local_cleanup (_this, _local);    \
                GF_FREE (_local);                       \
        } while (0)

#define QUOTA_SAFE_INCREMENT(lock, var)         \
        do {                                    \
                LOCK (lock);                    \
                var ++;                         \
                UNLOCK (lock);                  \
        } while (0)

#define QUOTA_SAFE_DECREMENT(lock, var)         \
        do {                                    \
                LOCK (lock);                    \
                var --;                         \
                UNLOCK (lock);                  \
        } while (0)

#define QUOTA_LOCAL_ALLOC_OR_GOTO(local, type, label)           \
        do {                                                    \
                QUOTA_ALLOC_OR_GOTO (local, type, label);       \
                LOCK_INIT (&local->lock);                       \
        } while (0)

#define QUOTA_ALLOC_OR_GOTO(var, type, label)           \
        do {                                            \
                var = GF_CALLOC (sizeof (type), 1,      \
                                 gf_quota_mt_##type);   \
                if (!var) {                             \
                        gf_log ("", GF_LOG_ERROR,       \
                                "out of memory :(");    \
                        ret = -1;                       \
                        goto label;                     \
                }                                       \
        } while (0);

#define QUOTA_STACK_UNWIND(fop, frame, params...)                       \
        do {                                                            \
                quota_local_t *_local = NULL;                           \
                xlator_t      *_this  = NULL;                           \
                if (frame) {                                            \
                        _local = frame->local;                          \
                        _this  = frame->this;                           \
                        frame->local = NULL;                            \
                }                                                       \
                STACK_UNWIND_STRICT (fop, frame, params);               \
                quota_local_cleanup (_this, _local);                    \
                GF_FREE (_local);                                       \
        } while (0)

#define QUOTA_FREE_CONTRIBUTION_NODE(_contribution)     \
        do {                                            \
                list_del (&_contribution->contri_list); \
                GF_FREE (_contribution);                \
        } while (0)

#define GET_CONTRI_KEY(var, _vol_name, _gfid, _ret)             \
        do {                                                    \
                char _gfid_unparsed[40];                        \
                uuid_unparse (_gfid, _gfid_unparsed);           \
                _ret = gf_asprintf (var, QUOTA_XATTR_PREFIX     \
                                    "%s.%s." CONTRIBUTION,      \
                                    _vol_name, _gfid_unparsed); \
        } while (0)


#define GET_CONTRI_KEY_OR_GOTO(var, _vol_name, _gfid, label)    \
        do {                                                    \
                GET_CONTRI_KEY(var, _vol_name, _gfid, ret);     \
                if (ret == -1)                                  \
                        goto label;                             \
        } while (0)

#define GET_DIRTY_KEY_OR_GOTO(var, _vol_name, label)            \
        do {                                                    \
                ret = gf_asprintf (var, QUOTA_XATTR_PREFIX      \
                                   "%s." DIRTY, _vol_name);     \
                if (ret == -1)                                  \
                        goto label;                             \
        } while (0)

struct quota_dentry {
        char            *name;
        ino_t            par;
        struct list_head next;
};
typedef struct quota_dentry quota_dentry_t;

struct quota_inode_ctx {
        int64_t          size;
        int64_t          limit;
        struct iatt      buf;
        char             just_validated;
        struct list_head parents;
        struct timeval   tv;
        gf_lock_t        lock;
};
typedef struct quota_inode_ctx quota_inode_ctx_t;

struct quota_local {
        gf_lock_t    lock;
        uint32_t     validate_count;
        uint32_t     link_count;
        loc_t        loc;
        loc_t        oldloc;
        loc_t        newloc;
        loc_t        validate_loc;
        uint32_t     delta;
        int32_t      op_ret;
        int32_t      op_errno;
        int64_t      size;
        int64_t      limit;
        inode_t     *inode;
        call_stub_t *stub;
};
typedef struct quota_local quota_local_t;

struct quota_priv {
        int64_t  timeout;
        struct list_head limit_head;
};
typedef struct quota_priv quota_priv_t;

struct limits {
        struct list_head  limit_list;
        char             *path;
        int64_t          value;
};
typedef struct limits limits_t;

uint64_t cn = 1;
