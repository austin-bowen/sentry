#pragma once

#include <Arduino.h>


namespace Async {
  class AsyncFunc {
    public:
      AsyncFunc *prev = nullptr;
      AsyncFunc *next = nullptr;

      void (*callback)();
      unsigned long period;

      long remaining_runs;
      unsigned long last_t;
      bool is_running = false;

      AsyncFunc(void (*callback)(), unsigned long period, long num_runs);

      bool IsReadyToRun();

      bool HasRunsRemaining();

      void Run();
  };


  class Async {
    public:
      ~Async();

      /** Run the given callback once after ms milliseconds. */
      void RunOnce(unsigned long ms, void (*callback)());

      /** Run the given callback periodically every ms milliseconds. */
      void RunForever(unsigned long ms, void (*callback)());

      void RunNumTimes(long num_runs, unsigned long ms, void (*callback)());

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

      // TODO: void SetEnabled(unsigned long id, bool enabled);

      // TODO: void RemoveFunc(unsigned long id);

      void RemoveAll();

      /** Returns the fraction of time [0..1] spent running callback functions.*/
      float GetLoad();

    protected:
      AsyncFunc *current_func_ = nullptr;
      unsigned long func_count_ = 0;

      unsigned long time_running_ = 0;
      unsigned long total_time_ = 0;

      void HandleOne();

      void RemoveFunc(AsyncFunc *func);

      bool HasFuncs() { return current_func_ != nullptr; }
      void NextFunc() { current_func_ = current_func_->next; }
  };
}
