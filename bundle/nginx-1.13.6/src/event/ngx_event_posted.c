
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_queue_t  ngx_posted_accept_events;
ngx_queue_t  ngx_posted_events;
ngx_queue_t  ngx_posted_delayed_events;

/* yanqi */
#include <sys/types.h>
#include <unistd.h>

static inline uint64_t rdtsc(void);

static inline uint64_t rdtsc(void)
{
    uint64_t var;
    uint32_t hi, lo;

    __asm volatile
        ("rdtsc" : "=a" (lo), "=d" (hi));

    var = ((uint64_t)hi << 32) | lo;
    return (var);
}

void
ngx_event_process_posted(ngx_cycle_t *cycle, ngx_queue_t *posted)
{
    ngx_queue_t  *q;
    ngx_event_t  *ev;

    // yanqi
    unsigned start_time;
    unsigned end_time;
    unsigned loop_start_time;

    while (!ngx_queue_empty(posted)) {

        q = ngx_queue_head(posted);
        ev = ngx_queue_data(q, ngx_event_t, queue);

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                      "posted event %p", ev);

        ngx_delete_posted_event(ev);

        // yanqi
        start_time = rdtsc();

        ev->handler(ev);

        // yanqi
        end_time = rdtsc();
        ngx_log_error(NGX_LOG_INFO, cycle->log, 0,
                          "ev_posted->handler() cycle %d, pid %d\n", end_time - start_time, getpid());

        ngx_log_error(NGX_LOG_INFO, cycle->log, 0,
                          "ngx_event_process_posted queueing cycle %d, pid %d\n", end_time - loop_start_time, getpid());
    }
}
