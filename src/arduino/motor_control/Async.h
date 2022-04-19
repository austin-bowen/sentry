#pragma once

#include <Arduino.h>
#include <Print.h>


namespace Async {
  using FuncId = unsigned long;

  class AsyncFunc {
    public:
      AsyncFunc *prev = nullptr;
      AsyncFunc *next = nullptr;

      FuncId id;
      void (*callback)();
      unsigned long period;
      long remaining_runs;

      unsigned long last_t;
      unsigned long run_time = 0L;
      bool is_enabled = true;
      bool is_running = false;

      AsyncFunc(FuncId id, void (*callback)(), unsigned long period, long num_runs);

      void SetEnabled(bool enabled) { is_enabled = enabled; }

      bool IsReadyToRun();

      bool HasRunsRemaining();

      void Run();
  };


  class Async {
    public:
      ~Async();

      /** Run the given callback once after ms milliseconds. */
      FuncId RunOnce(unsigned long ms, void (*callback)());

      /** Run the given callback periodically every ms milliseconds. */
      FuncId RunForever(unsigned long ms, void (*callback)());

      FuncId RunNumTimes(long num_runs, unsigned long ms, void (*callback)());

      /** Should be called once per loop. Gives each callback a chance to run. */
      void Handle();

      /**
       * Meant to be used by a callback function to delay its execution like the
       * built-in delay function, but without blocking the other callbacks.
       */
      void Delay(unsigned long ms);

      void DelayMicroseconds(unsigned int us);

      /**
       * Meant to be used by a (possibly long-running) callback function
       * to give other callbacks a chance to run.
       */
      void Yield() { Handle(); }

      AsyncFunc *GetFunc(FuncId id);

      void RemoveFunc(FuncId id);

      void RemoveAll();

      /** Returns the fraction of time [0..1] spent running callback functions.*/
      float GetLoad();

      void PrintStats(Print &printer);

    protected:
      AsyncFunc *current_func_ = nullptr;
      unsigned long func_count_ = 0;

      FuncId next_id_ = 0;

      unsigned long time_running_ = 0;
      unsigned long total_time_ = 0;

      void HandleOne();

      void RemoveFunc(AsyncFunc *func);

      bool HasFuncs() { return current_func_ != nullptr; }
      void NextFunc() { current_func_ = current_func_->next; }
      FuncId GetNextId();
  };
}
