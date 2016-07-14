#ifndef PWM_H_
#define PWM_H_

class PWMDriver {
  int m_pin;
  int m_value;
  int m_deadZone;
  
  public:
  /** 
   * Attach the driver to a specified pin. With a default value.
   */
  PWMDriver(const int &pin, const int &value = 0, const int& deadZone = 0) { 
    m_pin = pin;
    setDeadZone(deadZone);
  }

  void setDeadZone(const int &deadZone) {
    m_deadZone = constrain(deadZone, 0, 255);
  }
  
  void set(const int &value) {
    m_value = value == 0 ? 0 : map(value, 0, 255, m_deadZone, 255);  
    update();
  }

  void update() {
    analogWrite(m_pin, m_value);
  }
};

#endif // PWM_H_
