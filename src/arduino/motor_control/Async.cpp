#include <Arduino.h>

#include "Async.h"


namespace Async {
  AsyncFunc::AsyncFunc(void (*callback)(), unsigned long period, long num_runs) {
    this->callback = callback;
    this->period = period;
    this->remaining_runs = num_runs;

    last_t = millis();
  }

  bool AsyncFunc::IsReadyToRun() {
    return !is_running && (millis() - last_t) >= period;
  }

  bool AsyncFunc::HasRunsRemaining() {
    return remaining_runs != 0;
  }

  void AsyncFunc::Run() {
    last_t = millis();

    is_running = true;
    callback();
    is_running = false;

    if (remaining_runs > 0) {
      remaining_runs--;
    }
  }

  Async::~Async() {
    RemoveAll();
  }

  void Async::RunOnce(unsigned long ms, void (*callback)()) {
    RunNumTimes(1, ms, callback);
  }

  void Async::RunForever(unsigned long ms, void (*callback)()) {
    RunNumTimes(-1, ms, callback);
  }

  void Async::RunNumTimes(long num_runs, unsigned long ms, void (*callback)()) {
    AsyncFunc *new_func = new AsyncFunc(callback, ms, num_runs);

    if (current_func_ == nullptr) {
      current_func_ = new_func->prev = new_func->next = new_func;
    } else {
      current_func_->prev->next = new_func;
      new_func->prev = current_func_->prev;

      current_func_->prev = new_func;
      new_func->next = current_func_;
    }

    func_count_++;
  }

  void Async::RemoveAll() {
    while (HasFuncs()) {
      RemoveFunc(current_func_);
    }
  }

  void Async::Handle() {
    for (unsigned long i = 0; i < func_count_; i++) {
      HandleOne();
    }
  }

  void Async::Delay(unsigned long ms) {
    unsigned long start_t = millis();

    while ((millis() - start_t) < ms) {
      HandleOne();
    }
  }

  void Async::DelayMicroseconds(unsigned int us) {
    unsigned long start_t = micros();

    while ((micros() - start_t) < us) {
      HandleOne();
    }
  }

  void Async::HandleOne() {
    if (!HasFuncs()) {
      return;
    }

    if (!current_func_->IsReadyToRun()) {
      NextFunc();
      return;
    }

    current_func_->Run();

    // All funcs could have been removed
    if (!HasFuncs()) {
      return;
    }

    // Do not need to remove current func?
    if (current_func_->HasRunsRemaining()) {
      NextFunc();
      return;
    }

    RemoveFunc(current_func_);

    if (HasFuncs()) {
      NextFunc();
    }
  }

  void Async::RemoveFunc(AsyncFunc *func) {
    if (func_count_ > 1) {
      if (func == current_func_) {
        current_func_ = func->prev;
      }

      func->prev->next = func->next;
      func->next->prev = func->prev;
    } else {
      current_func_ = nullptr;
    }

    delete func;
    func_count_--;
  }
}
