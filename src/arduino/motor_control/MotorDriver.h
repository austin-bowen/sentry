#ifndef MOTOR_CONTROL_MOTOR_DRIVER_H_
#define MOTOR_CONTROL_MOTOR_DRIVER_H_

class MotorDriver {
  public:
    MotorDriver(int pwm_pin, int dir_pin);

    /**
     * Set the power delivered to the motor.
     *
     * Range is [-1, 1], where 1 is 100% power forward (dir pin low),
     * 0 is stopped, and -1 is 100% power in reverse (dir pin high).
     */
    void SetPower(float power);

    void Stop();
    
    void Reverse(bool reverse);

    void InvertPowerOnReverse(bool invert);

  private:
    static const int MAX_PWM = 255;

    int pwm_pin_;
    int dir_pin_;

    bool reverse_ = false;
    bool invert_power_on_reverse_ = true;
};

#endif
