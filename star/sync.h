
#ifndef STAR_SYNC_H
#define STAR_SYNC_H

/**
 * @brief 包含了协程同步原语
 * CoMutex      协程锁
 * CoCondvar    协程条件变量
 * CoSemaphore  协程信号量
 * Channel      消息通道
 */

#include "sync/channel.h"
#include "sync/co_condvar.h"
#include "sync/co_semaphore.h"
#include "sync/mutex.h"

#endif //STAR_SYNC_H
