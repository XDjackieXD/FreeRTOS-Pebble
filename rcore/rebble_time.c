/* rebble_time.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "rebbleos.h"
#include "strftime.h"

static TickType_t _sync_ticks;
static time_t _sync_time_t;


void rcore_time_init(void)
{
    _sync_time_rtc();
}

time_t rcore_mktime(struct tm *tm)
{
    return mktime(tm);
}

void rcore_localtime(struct tm *tm, time_t time)
{
    localtime_r(&time, tm);
}

void rcore_time_ms(time_t *tutc, uint16_t *ms)
{
    TickType_t ticks_since_sync = xTaskGetTickCount() - _sync_ticks;
    
    *tutc = _sync_time_t + ticks_since_sync / configTICK_RATE_HZ;
    *ms = (ticks_since_sync % configTICK_RATE_HZ) * 1000 / configTICK_RATE_HZ;
}

TickType_t rcore_time_to_ticks(time_t t, uint16_t ms) {
    if (t < _sync_time_t)
        return 0;
    return (t - _sync_time_t) * configTICK_RATE_HZ + pdMS_TO_TICKS(ms);
}

size_t rcore_strftime(char* buffer, size_t maxSize, const char* format, const struct tm* tm) {
    return strftime(buffer, maxSize, format, tm);
}

/*
 * Get the time as a tm struct
 */
/* XXX need one struct tm per app */
static struct tm _global_tm;
struct tm *rebble_time_get_tm(void)
{
    time_t tm;
    rcore_time_ms(&tm, NULL);
    rcore_localtime(&_global_tm, tm);
    return &_global_tm;
}

// Sync time and set RTC
void rebble_time_set_tm(struct tm *time_now)
{
    _sync_ticks = xTaskGetTickCount();
    _sync_time_t = rcore_mktime(time_now);

    hw_set_time(time_now);
}

// Read the current time from the RTC
void _sync_time_rtc(void)
{
    struct tm *tm;

    /* Read the time out of the RTC, then convert to a time_t (ugh!), then
     * begin offsetting ticks in ms from there.  */
    _sync_ticks = xTaskGetTickCount();
    tm = hw_get_time();
    _sync_time_t = rcore_mktime(tm);
}




uint16_t pbl_time_deprecated(time_t *tloc)
{
    /* XXX time zones: utc vs local time */
    uint16_t _ms;
    time_t _tm;
    
    rcore_time_ms(&_tm, &_ms);
    if (tloc)
        *tloc = _tm;
    
    return _ms;
}

uint16_t pbl_time_ms_deprecated(time_t *tloc, uint16_t *ms)
{
    /* XXX time zones: utc vs local time */
    uint16_t _ms;
    time_t _tm;
    
    rcore_time_ms(&_tm, &_ms);
    if (tloc)
        *tloc = _tm;
    if (ms)
        *ms = _ms;
    
    return _ms;
}

int pbl_clock_is_24h_style()
{
    // XXX: Obviously, everybody wants 24h time.  Why would they use a
    // developer operating system if not?
    return 1;
}
