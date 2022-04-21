#include <Arduino.h>

#include "Async.h"


namespace Async {
  AsyncFunc::AsyncFunc(FuncId id, void (*callback)(), unsigned long period, long num_runs) {
    this->id = id;
    this->callback = callback;
    this->period = period;
    this->remaining_runs = num_runs;

    last_t = millis();
  }

  bool AsyncFunc::IsReadyToRun() {
    return is_enabled && !is_running && (millis() - last_t) >= period;
  }

  bool AsyncFunc::HasRunsRemaining() {
    return remaining_runs != 0;
  }

  void AsyncFunc::Run() {
    unsigned long start_t = millis();

    is_running = true;
    callback();
    is_running = false;

    unsigned long stop_t = millis();

    if (remaining_runs > 0) {
      remaining_runs--;
    }

    // TODO: If the callback uses async.Delay(), it will be added to the runtime,
    //  which gives a skewed result for how long it actually took to execute.
    run_time = stop_t - start_t;

    if ((stop_t - last_t) > 2 * period) {
      misses++;
    }

    last_t = start_t;
  }

  Async::~Async() {
    RemoveAll();
  }

  FuncId Async::RunOnce(unsigned long ms, void (*callback)()) {
    return RunNumTimes(1, ms, callback);
  }

  FuncId Async::RunForever(unsigned long ms, void (*callback)()) {
    return RunNumTimes(AsyncFunc::FOREVER, ms, callback);
  }

  FuncId Async::RunNumTimes(long num_runs, unsigned long ms, void (*callback)()) {
    FuncId id = GetNextId();
    AsyncFunc *new_func = new AsyncFunc(id, callback, ms, num_runs);

    if (current_func_ == nullptr) {
      current_func_ = new_func->prev = new_func->next = new_func;
    } else {
      current_func_->prev->next = new_func;
      new_func->prev = current_func_->prev;

      current_func_->prev = new_func;
      new_func->next = current_func_;
    }

    func_count_++;

    return id;
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

  AsyncFunc *Async::GetFunc(FuncId id) {
    if (!HasFuncs()) {
      return nullptr;
    }

    AsyncFunc *func = current_func_;
    do {
      if (func->id == id) {
        return func;
      }

      func = func->next;
    } while (func != current_func_);

    return nullptr;
  }

  void Async::RemoveFunc(FuncId id) {
    AsyncFunc *func = GetFunc(id);
    if (func != nullptr) {
      RemoveFunc(func);
    }
  }

  void Async::RemoveAll() {
    while (HasFuncs()) {
      RemoveFunc(current_func_);
    }
  }

  float Async::GetLoad() {
    // TODO: NOT THIS
    return (float)time_running_ / millis();
//    return (float)time_running_ / (float)total_time_;
  }

  void Async::PrintStats(Print &printer) {
    printer.println("id\t rem_runs\t period [ms]\t run_time [ms]\t % period\t misses");

    if (!HasFuncs()) {
      printer.println("[None]\n");
      return;
    }

    AsyncFunc *start_func = current_func_;
    unsigned long run_time_sum = 0;
    unsigned long min_period = current_func_->period;
    do {
      AsyncFunc *func = current_func_;

      // ID / name
      if (func->name != nullptr) {
        printer.print(func->name);
      } else {
        printer.print(func->id);
      }
      printer.print("\t ");

      // Remaining runs
      if (func->remaining_runs == AsyncFunc::FOREVER) {
        printer.print("inf");
      } else {
        printer.print(func->remaining_runs);
      }
      printer.print("\t\t ");

      // Period
      printer.print(func->period);
      printer.print("\t\t ");

      // Run time
      printer.print(func->run_time);
      printer.print("\t\t ");

      // Run time % of period
      printer.print(100.f * func->run_time / func->period);
      printer.print("\t\t ");

      // Misses
      printer.print(func->misses);
      printer.println();

      run_time_sum += func->run_time;
      min_period = min(min_period, func->period);

      NextFunc();
    } while (current_func_ != start_func);

    if (run_time_sum > min_period) {
      printer.print("\nWARNING: Total run time (");
      printer.print(run_time_sum);
      printer.print(" ms) > min period (");
      printer.print(min_period);
      printer.print(" ms); misses may occur!\n");
    }

    printer.println();
  }

  void Async::HandleOne() {
    if (!HasFuncs()) {
      return;
    }

    if (!current_func_->IsReadyToRun()) {
      NextFunc();
      return;
    }

    unsigned long t0 = millis();
    current_func_->Run();
    time_running_ += millis() - t0;

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

  FuncId Async::GetNextId() {
    FuncId id;
    do {
      id = next_id_++;
    } while (GetFunc(id) != nullptr);

    return id;
  }
}
