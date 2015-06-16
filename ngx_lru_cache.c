/**
 * @file ngx_lru_cache.c
 * @author mian_sun(at)foxmail.com
 * @date 2015/06/16 22:47:09
 * @brief
 *
 **/

#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_lru_cache.h"

static ngx_rbtree_node_t* ngx_rbtree_lookup(ngx_rbtree_t* rbtree, ngx_rbtree_node_t* sentinel,
        ngx_rbtree_key_t key);

static ngx_inline void ngx_lru_cache_old_expires(ngx_lru_cache_t* cache);

static ngx_inline void
ngx_lru_cache_old_expires(ngx_lru_cache_t* cache)
{
    time_t       now = ngx_time();
    ngx_queue_t* q = ngx_queue_last(cache->queue);
    for (; q != ngx_queue_sentinel(cache->queue); q = ngx_queue_prev(q)) {
        ngx_lru_cache_node_t* node = ngx_queue_data(q, ngx_lru_cache_node_t, queue_node);
        if (now - node->accessed > cache->expires_time) {
            ngx_queue_remove(&node->queue_node);
            ngx_rbtree_delete(&cache->rbtree, (ngx_rbtree_node_t*)node);
            --cache->size;
        } else {
            break;
        }
    }
}

ngx_lru_cache_t*
ngx_lru_cache_create(ngx_pool_t* p, time_t expires_time)
{
    ngx_lru_cache_t* cache;

    if (p == NULL || expires_time <= 0) {
        return NULL;
    }

    cache = ngx_palloc(p, sizeof(ngx_lru_cache_t));
    if (cache == NULL) {
        return NULL;
    }

    cache->expires_time = expires_time;

    cache->queue = ngx_palloc(p, sizeof(ngx_queue_t));  //alloc a queue head
    if (cache->queue == NULL) {
        return NULL;
    }
    ngx_queue_init(cache->queue);  //init the queue

    cache->pool = p;
    cache->size = 0;

    return cache;
}

uintptr_t
ngx_lru_cache_get(ngx_lru_cache_t* cache, ngx_rbtree_key_t key)
{
    time_t    now = ngx_time();
    uintptr_t value = 0;
    ngx_lru_cache_old_expires(cache);
    ngx_lru_cache_node_t* node = (ngx_lru_cache_node_t*)ngx_rbtree_lookup(&cache->rbtree,
                                 &cache->sentinel, key);

    if (node != NULL) {
        ngx_queue_remove(&node->queue_node);
        ngx_queue_insert_head(cache->queue, &node->queue_node);
        node->accessed = now;
        value = node->data;
    }
    return value;
}

ngx_int_t
ngx_lru_cache_set(ngx_lru_cache_t* cache, ngx_rbtree_key_t key, uintptr_t value)
{
    time_t now = ngx_time();
    ngx_lru_cache_old_expires(cache);
    ngx_lru_cache_node_t* node = (ngx_lru_cache_node_t*)ngx_rbtree_lookup(&cache->rbtree,
                                 &cache->sentinel, key);
    if (node == NULL) {
        node = ngx_palloc(cache->pool, sizeof(ngx_lru_cache_node_t));
        if (node == NULL) {
            return NGX_ERROR;
        }
        node->rbtree_node.key = key;
        ngx_rbtree_insert(&cache->rbtree, (ngx_rbtree_node_t*)node);
        ++cache->size;
    } else {
        ngx_queue_remove(&node->queue_node);
    }
    node->accessed = now;
    node->data = value;
    ngx_queue_insert_head(cache->queue, &node->queue_node);
    return NGX_OK;
}

static ngx_rbtree_node_t*
ngx_rbtree_lookup(ngx_rbtree_t* rbtree, ngx_rbtree_node_t* sentinel,
                  ngx_rbtree_key_t key)
{
    ngx_rbtree_node_t* node = rbtree->root;
    while (node != sentinel) {
        if (key == node->key) {
            return node;
        }
        node = (key < node->key) ? node->left : node->right;
    }
    return NULL;
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
