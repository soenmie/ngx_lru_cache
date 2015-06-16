/**
 * @file ngx_lru_cache.h
 * @author mian_sun(at)foxmail.com
 * @date 2015/06/17 01:25:03
 * @brief
 *
 **/

#ifndef _NGX_LRU_CACHE_H_INCLUDED_
#define _NGX_LRU_CACHE_H_INCLUDED_

typedef struct ngx_lru_cache_node_s ngx_lru_cache_node_t;

struct ngx_lru_cache_node_s {
    ngx_rbtree_node_t   rbtree_node;
    ngx_queue_t         queue_node;
    time_t              accessed;
    uintptr_t           data;
};

typedef struct ngx_lru_cache_s ngx_lru_cache_t;

struct ngx_lru_cache_s {
    ngx_rbtree_t         rbtree;
    ngx_rbtree_node_t    sentinel;
    ngx_queue_t*         queue;
    time_t               expires_time;
    ngx_pool_t*          pool;
    size_t               size;
};

ngx_lru_cache_t* ngx_lru_cache_create(ngx_pool_t* p, time_t expires_time);

uintptr_t ngx_lru_cache_get(ngx_lru_cache_t* cache, ngx_rbtree_key_t key);

ngx_int_t ngx_lru_cache_set(ngx_lru_cache_t* cache, ngx_rbtree_key_t key, uintptr_t value);

#endif  /* _NGX_LRU_CACHE_H_INCLUDED_ */

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
