
class PWMDriver {
  public:
  // Works only for 11 and 3
  PWMDriver() { }
  
  void set(int pin, int value) {
      analogWrite(pin, value);
  }
};

