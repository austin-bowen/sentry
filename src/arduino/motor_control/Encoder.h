#pragma once


class Encoder {
  public:
    volatile long ticks = 0L;

    long GetTicks() { return ticks; }
    void SetTicks(long ticks) { this->ticks = ticks; }
};


class QuadratureEncoder : public Encoder {
  public:
    QuadratureEncoder(int enc_a_pin, int enc_b_pin, unsigned int ticks_per_rev);

    /**
     * These handlers should be functions that simply call this encoder's
     * HandleEncAChange() and HandleEncBChange() methods, respectively.
     */
    void SetupEncPinChangeHandlers(void (*enc_a_handler)(), void (*enc_b_handler)());

    void HandleEncAChange();
    void HandleEncBChange();

    long GetTicksPerRev() { return ticks_per_rev_; }

    void Reverse(bool reverse);

  private:
    int enc_a_pin_;
    int enc_b_pin_;
    unsigned int ticks_per_rev_;

    int tick_increment_ = 1;
};
