#ifndef FSLOG_BASE_ATOMIC_H
#define FSLOG_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

template<typename T>
class AtomicInterger : boost::noncopyable
{
 public:
  AtomicInterger()
    : value_(0)
  {
  }

  T compareAndExchange(T compareValue, T exchangeValue)
  {
      return __sync_val_compare_and_swap(&value_, compareValue, exchangeValue);
  }

  T getAndSet(T newValue)
  {
    return __sync_lock_test_and_set(&value_, newValue);
  }

  T getAndAdd(T x)
  {
    return __sync_fetch_and_add(&value_, x);
  }

  T getAndSub(T x)
  {
    return __sync_fetch_and_sub(&value_, x);
  }

  T addAndGet(T x)
  {
      return __sync_add_and_fetch(&value_, x);
  }

  T subAndGet(T x)
  {
      return __sync_sub_and_fetch(&value_, x);
  }

  T get()
  {
    return compareAndExchange(0, 0);
  }
  
  T increment()
  {
    return addAndGet(1);
  }

  T decrement()
  {
    return subAndGet(1);
  }
  
 private:
  volatile T value_;
};


typedef AtomicInterger<int32_t> AtomicInt32;
typedef AtomicInterger<int64_t> AtomicInt64;

#endif
