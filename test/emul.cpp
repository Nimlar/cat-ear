#include <cstddef>
#include <iostream>
#include <chrono>

class WiFiServer {
    /* only a stub */
public:
    WiFiServer() { }
    WiFiServer(int port) { (void)port; }
};

const char * pin_name(int pin);

class Servo {
    bool _attached;
    int _pin;
public:
    Servo(): _attached(false), _pin(-1) { }
    bool attached() { return this->_attached ;}
    int write(int angle) { std::cout << "   " << pin_name(this->_pin) << "(" << this->_attached <<") move to " << angle <<" angle" << std::endl ; return angle; }

    void attach(int pin) { this->_pin = pin; this->_attached=true; }
    void detach() { this->_attached=false; }
};
unsigned long millis(void)
{
    static unsigned long start = 0;
    if (!start) {
        start = std::chrono::system_clock::now().time_since_epoch().count();
    }
    return (std::chrono::system_clock::now().time_since_epoch().count() - start );
}

#define PC_EMUL 1
#define P(...) printf( __VA_ARGS__ )
#define Ppin(pin, str, ...) printf("%s " str, pin_name(pin), ##__VA_ARGS__ )

#include "../cat-ear.ino"

const char * pin_name(int pin) 
{
switch ( pin ) {
    case LeftAltPin : return "Left Alt";
    case LeftAziPin : return "Left Azi";
    case RightAltPin: return "RightAlt";
    case RightAziPin: return "RightAzi";
    default:          return "unknown";
    }
}


int main(void)
{
#if 0
    std::cout << "mv_triste" << std::endl ;
    mvt_triste();
    while(!ears.step());
    std::cout << std::endl<< std::endl  ;
#endif
#if 1
    std::cout << "mvt_right" << std::endl ;
    mvt_droit();
    while(!ears.step()) {

    };
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mvt_gauche" << std::endl ;
    mvt_gauche();
    while(!ears.step());
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mv_penaud" << std::endl ;
    mvt_penaud();
    while(!ears.step());
    std::cout << std::endl<< std::endl  ;
#endif

    return 0;
}
