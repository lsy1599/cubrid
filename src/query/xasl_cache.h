/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

/*
 * XASL cache.
 */

#ifndef _XASL_CACHE_H_
#define _XASL_CACHE_H_

#ident "$Id$"

#if !defined (SERVER_MODE) && !defined (SA_MODE)
#error Belongs to server module
#endif /* !defined (SERVER_MODE) && !defined (SA_MODE) */

#include "xasl.h"

/* Objects related to XASL cache entries. The information includes the object OID, the lock required to use the XASL
 * cache entry and the heap file cardinality.
 * Objects can be classes or serials. The heap file cardinality is only relevant for classes.
 */
typedef struct xcache_related_object XCACHE_RELATED_OBJECT;
struct xcache_related_object
{
  OID oid;
  LOCK lock;
  int tcard;
};

enum xcache_cleanup_reason
{
  XCACHE_CLEANUP_NONE,		/* no cleanup is required */
  XCACHE_CLEANUP_FULL,
  XCACHE_CLEANUP_TIMEOUT
};
typedef enum xcache_cleanup_reason XCACHE_CLEANUP_REASON;

/* XASL cache clones - XASL nodes cached for fast usage.
 *
 */
typedef struct xasl_clone XASL_CLONE;
struct xasl_clone
{
  void *xasl_buf;		/* TODO: Make XASL_UNPACK_INFO visible. */
  XASL_NODE *xasl;
};
#define XASL_CLONE_INITIALIZER { NULL, NULL }
#define XASL_CLONE_AS_ARGS(clone) (clone)->xasl, (clone)->xasl_buf

/*
 * EXECUTION_INFO: query strings: user text, hash string and dumped plan.
 */
typedef struct execution_info EXECUTION_INFO;
struct execution_info
{
  char *sql_hash_text;		/* rewritten query string which is used as hash key */
  char *sql_user_text;		/* original query statement that user input */
  char *sql_plan_text;		/* plans for this query */
};
#define EXEINFO_HASH_TEXT_STRING(einfo) ((einfo)->sql_hash_text ? (einfo)->sql_hash_text : "UNKNOWN HASH TEXT")
#define EXEINFO_USER_TEXT_STRING(einfo) ((einfo)->sql_user_text ? (einfo)->sql_user_text : "UNKNOWN USER TEXT")
#define EXEINFO_PLAN_TEXT_STRING(einfo) ((einfo)->sql_plan_text ? (einfo)->sql_plan_text : "UNKNOWN PLAN TEXT")

#define EXEINFO_AS_ARGS(einfo)	\
  EXEINFO_USER_TEXT_STRING(einfo), EXEINFO_PLAN_TEXT_STRING(einfo), EXEINFO_HASH_TEXT_STRING(einfo)

/* This really belongs more to the query manager rather than query executor. */
/* XASL cache entry type definition */
typedef struct xasl_cache_ent XASL_CACHE_ENTRY;
struct xasl_cache_ent
{
  XASL_ID xasl_id;		/* XASL file identifier */
  /* Not really a file identifier anymore. Maybe we should rename this to XASL_KEY? */
  XASL_STREAM stream;		/* XASL packed stream. */

  /* Latch-free stuff. */
  XASL_CACHE_ENTRY *stack;	/* used in freelist */
  XASL_CACHE_ENTRY *next;	/* used in hash table */
  /* Mutex may be addex here (hopefully it is not necessary). */
  UINT64 del_id;		/* delete transaction ID (for lock free) */


  EXECUTION_INFO sql_info;	/* cache entry hash key, user input string & plan */
  int xasl_header_flag;		/* XASL header info */
  XCACHE_RELATED_OBJECT *related_objects;	/* List of objects referenced by XASL cache entry.
						 * Objects can be:
						 * 1. classes
						 * 2. serials
						 */
  int n_related_objects;	/* size of the related_objects */
  struct timeval time_last_used;	/* when this entry used lastly */
  INT64 ref_count;		/* how many times this entry used */
  int list_ht_no;		/* memory hash table for query result(list file) cache generated by this XASL
				 * referencing by DB_VALUE parameters bound to the result */
  bool free_data_on_uninit;	/* set to free entry data on uninit. */

  /* Cache clones */
  XASL_CLONE *cache_clones;
  XASL_CLONE one_clone;
  int n_cache_clones;
  int cache_clones_capacity;
  pthread_mutex_t cache_clones_mutex;

  /* RT check */
  INT64 time_last_rt_check;

  bool initialized;
};

extern int xcache_initialize (THREAD_ENTRY * thread_p);
extern void xcache_finalize (THREAD_ENTRY * thread_p);

extern int xcache_find_sha1 (THREAD_ENTRY * thread_p, const SHA1Hash * sha1, XASL_CACHE_ENTRY ** xcache_entry,
			     bool * rt_check);
extern int xcache_find_xasl_id (THREAD_ENTRY * thread_p, const XASL_ID * xid, XASL_CACHE_ENTRY ** xcache_entry,
				XASL_CLONE * xclone);
extern void xcache_unfix (THREAD_ENTRY * thread_p, XASL_CACHE_ENTRY * xcache_entry);
extern int xcache_insert (THREAD_ENTRY * thread_p, const COMPILE_CONTEXT * context, XASL_STREAM * stream,
			  int n_oid, const OID * class_oids, const int *class_locks,
			  const int *tcards, XASL_CACHE_ENTRY ** xcache_entry);
extern void xcache_remove_by_oid (THREAD_ENTRY * thread_p, OID * oid);
extern void xcache_drop_all (THREAD_ENTRY * thread_p);
extern void xcache_dump (THREAD_ENTRY * thread_p, FILE * fp);

extern bool xcache_can_entry_cache_list (XASL_CACHE_ENTRY * xcache_entry);

extern void xcache_retire_clone (THREAD_ENTRY * thread_p, XASL_CACHE_ENTRY * xcache_entry, XASL_CLONE * xclone);
extern int xcache_get_entry_count (void);
extern bool xcache_uses_clones (void);
#endif /* _XASL_CACHE_H_ */
