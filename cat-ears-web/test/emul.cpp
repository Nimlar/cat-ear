#include <cstddef>
#include <iostream>
#include <chrono>

#define byte int

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
    int write(int angle) { return angle; }
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

class OldDef {
    const char *_name;
    int _pin;
    int _neuter;
public:
    OldDef(const char * name, int neuter): _name(name), _neuter(neuter) {}
    void attach(int pin) { this->_pin = pin; P("###"); Ppin(pin, "Attach\n");  }
    void detach() {P("###"); Ppin(this->_pin, "Detach\n"); }
    void write(int angle) { std::cout << "###" << this->_name << " write " << angle << " angle (" << angle - this->_neuter << ")" << std::endl ; }
};


#define     LeftVerPin           LeftAltPin
#define     LeftAngPin           LeftAziPin
#define     RightVerPin          RightAltPin
#define     RightAngPin          RightAziPin


#define     INIT_LEFT_VER_POS    INIT_LEFT_ALT
#define     INIT_RIGHT_VER_POS   INIT_RIGHT_ALT
#define     INIT_LEFT_ANG_POS    INIT_LEFT_AZI
#define     INIT_RIGHT_ANG_POS   INIT_RIGHT_AZI


void delay(int wait) {
std::cout << "###" << "will wait " << wait << std::endl;
}

static OldDef LeftVer("Left Alt", INIT_LEFT_ALT);
static OldDef RightVer("RightAlt", INIT_RIGHT_ALT);
static OldDef LeftAng("Left Azi", INIT_LEFT_AZI);
static OldDef RightAng("RightAzi", INIT_RIGHT_AZI);

//1
void old_mvt_triste(void)
{
  const int MaxAngleShift = 30;
  unsigned int MoveDelay = 5;
  unsigned int PosDelay = 10000;

  std::cout << "###old triste" << std::endl  ;
  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);
  
  for(byte i=0; i <= MaxAngleShift; i++)
  {
    LeftVer.write(INIT_RIGHT_VER_POS + i);
    RightVer.write(INIT_LEFT_VER_POS - i);
    delay(MoveDelay);
  }
  
  for(byte i=0; i <= 3*MaxAngleShift; i++)
  {
    LeftVer.write(INIT_RIGHT_VER_POS + MaxAngleShift - i);
    RightVer.write(INIT_LEFT_VER_POS - MaxAngleShift + i);
    delay(3*MoveDelay);
  }
  
  delay(PosDelay);

  LeftVer.write(INIT_LEFT_VER_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  delay(500);

  LeftVer.detach();
  RightVer.detach();
}

//2
void old_mvt_penaud(void)
{
   const byte MaxAngleShift = 100;
  const byte MaxAngleShift2 = 20;
  unsigned int MoveDelay = 10;
  unsigned int PosDelay = 5000;

  std::cout << "###old penaud" << std::endl  ;
  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);
  
  for(byte i=0; i <= MaxAngleShift; i++)
  {
    LeftAng.write(INIT_LEFT_ANG_POS + i);
    RightAng.write(INIT_RIGHT_ANG_POS - i);
    delay(MoveDelay);
  }

  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);
  
  for(byte i=0; i <= MaxAngleShift2; i++)
  {
    RightVer.write(INIT_LEFT_VER_POS - i);
    LeftVer.write(INIT_RIGHT_VER_POS + i);
    delay(MoveDelay);
  }
  
  for(byte i=0; i <= 3*MaxAngleShift2; i++)
  {
    RightVer.write(INIT_LEFT_VER_POS - MaxAngleShift2 + i);
    LeftVer.write(INIT_RIGHT_VER_POS + MaxAngleShift2 - i);
    delay(3*MoveDelay);
  }
    delay(PosDelay);
  
  LeftVer.write(INIT_LEFT_VER_POS);
  LeftAng.write(INIT_LEFT_ANG_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  RightAng.write(INIT_RIGHT_ANG_POS);

    delay(300);
  

  LeftVer.detach();
  RightVer.detach();
  LeftAng.detach();
  RightAng.detach();
 
}

//3
void old_mvt_gauche(void)
{
	
  unsigned int MoveDelay = 10;
  byte AnglePos;
  
  std::cout << "###old gauche" << std::endl  ;
  LeftAng.attach(LeftAngPin);
  for(byte i=0; i <= 50; i++)
  {
    LeftAng.write(INIT_LEFT_ANG_POS + i);
    delay(3);
  }
  LeftAng.detach();

  AnglePos = -INIT_LEFT_VER_POS;
  LeftVer.attach(LeftVerPin);
  for(byte i=0; i <= 50; i++)
  {
    LeftVer.write(AnglePos);
    AnglePos = INIT_LEFT_VER_POS + i;
    delay(MoveDelay);
  }
   
  for(byte i=0; i <= 15; i++)
  {
    LeftVer.write(AnglePos);
    AnglePos -= i;
    delay(MoveDelay);
  }
      
  for(byte j=0; j<5; j++)
  {
    for(byte i=0; i <= 15; i++)
    {
      LeftVer.write(AnglePos);
      AnglePos += i;
      delay(MoveDelay);
    }
    
    for(byte i=0; i <= 15; i++)
    {
      LeftVer.write(AnglePos);
      AnglePos -= i;
      delay(MoveDelay);
    }
  }

  LeftAng.attach(LeftAngPin);
  LeftVer.write(INIT_LEFT_VER_POS);
  LeftAng.write(INIT_LEFT_ANG_POS);
  delay(500);

  LeftAng.detach();
  LeftVer.detach();
}

//4
void old_mvt_droit(void)
{
  unsigned int MoveDelay = 10;
  byte AnglePos=0;
  
  std::cout << "###old droit" << std::endl  ;
  RightAng.attach(RightAngPin);
  for(byte i=0; i <= 50; i++)
  {
    RightAng.write(INIT_RIGHT_ANG_POS - i);
    delay(3);
  }
  RightAng.detach();

  RightVer.attach(RightVerPin);
  for(byte i=0; i <= 50; i++)
  {
    AnglePos = INIT_RIGHT_VER_POS - i;
    RightVer.write(AnglePos);
    delay(MoveDelay);
  }
  
  for(byte i=0; i <= 15; i++)
  {
    RightVer.write(AnglePos);
    AnglePos += i;
    delay(MoveDelay);
  }
    
  for(byte j=0; j<5; j++)
  {
    for(byte i=0; i <= 15; i++)
    {
      RightVer.write(AnglePos);
      AnglePos -= i;
      delay(MoveDelay);
    }
    
    for(byte i=0; i <= 15; i++)
    {
      RightVer.write(AnglePos);
      AnglePos += i;
      delay(MoveDelay);
    }
  }
  
  RightAng.attach(RightAngPin);
  RightVer.write(INIT_RIGHT_VER_POS);
  RightAng.write(INIT_RIGHT_ANG_POS);
  delay(500);

  RightAng.detach();
  RightVer.detach();

}

//5
void old_mvt_aguet(void)
{
  unsigned int MoveDelay = 10;
  byte AngleLeftPos;
  byte AngleRightPos;

  std::cout << "###old aguet" << std::endl  ;
  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);
  for(byte i=0; i <= 50; i++)
  {
    LeftAng.write(INIT_LEFT_ANG_POS + i);
    RightAng.write(INIT_RIGHT_ANG_POS - i);
    delay(3);
  }
  LeftAng.detach();
  RightAng.detach();

  AngleLeftPos = INIT_LEFT_VER_POS;
  AngleRightPos = INIT_RIGHT_VER_POS;
  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);
  for(byte i=0; i <= 50; i++)
  {
    LeftVer.write(AngleLeftPos);
    RightVer.write(AngleRightPos);
    AngleLeftPos = INIT_LEFT_VER_POS + i;
    AngleRightPos = INIT_RIGHT_VER_POS - i;
    delay(MoveDelay);
  }

  for(byte i=0; i <= 10; i++)
  {
    LeftVer.write(AngleLeftPos);
    RightVer.write(AngleRightPos);
    AngleLeftPos -= i;
    AngleRightPos += i;
    delay(MoveDelay);
  }

  for(byte j=0; j<5; j++)
  {
    for(byte i=0; i <= 10; i++)
    {
      LeftVer.write(AngleLeftPos);
      RightVer.write(AngleRightPos);
      AngleLeftPos += i;
      AngleRightPos -= i;
      delay(MoveDelay);
    }
    
    for(byte i=0; i <= 10; i++)
    {
      LeftVer.write(AngleLeftPos);
      RightVer.write(AngleRightPos);
      AngleLeftPos -= i;
      AngleRightPos += i;
      delay(MoveDelay);
    }
  }

  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);
  LeftVer.write(INIT_LEFT_VER_POS);
  LeftAng.write(INIT_LEFT_ANG_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  RightAng.write(INIT_RIGHT_ANG_POS);
  delay(300);

  LeftAng.detach();
  LeftVer.detach();
  RightAng.detach();
  RightVer.detach();
}

//6
void old_mvt_content(void)
{
  const byte MaxAngleShift = 50;
  unsigned int PosDelay = 300;

  std::cout << "###old content" << std::endl  ;
  byte PosLeft = INIT_LEFT_VER_POS;
  byte PosRight = INIT_RIGHT_VER_POS;
  
  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);

  PosLeft -= MaxAngleShift;
  PosRight -= MaxAngleShift;
  LeftVer.write(PosLeft);
  RightVer.write(PosRight);

  delay(PosDelay);

  for(byte i = 0; i < 2; i++)
  {
    PosLeft += 2*MaxAngleShift;
    PosRight += 2*MaxAngleShift;
    LeftVer.write(PosLeft);
    RightVer.write(PosRight);
    delay(PosDelay);
    PosLeft -= 2*MaxAngleShift;
    PosRight -= 2*MaxAngleShift;
    LeftVer.write(PosLeft);
    RightVer.write(PosRight);
    delay(PosDelay);
  }

  LeftVer.write(INIT_LEFT_VER_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  delay(500);

  LeftVer.detach();
  RightVer.detach();


}

// 7
void old_mvt_ecoute(void)
{

  const byte MaxAngleShift = 100;
  unsigned int MoveDelay = 10;
  unsigned int PosDelay = 500;


  std::cout << "###old ecoute" << std::endl  ;
  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);
  
  for(byte i=0; i <= MaxAngleShift; i++)
  {
    LeftAng.write(INIT_LEFT_ANG_POS + i);
    RightAng.write(INIT_RIGHT_ANG_POS - i);
    delay(MoveDelay);
  }

  delay(300);

  for(byte i=0; i <= MaxAngleShift; i++)
  {
    LeftAng.write(INIT_LEFT_ANG_POS - i);
    RightAng.write(INIT_RIGHT_ANG_POS + i);
    delay(MoveDelay);
  }


  delay(300);

  LeftVer.write(INIT_LEFT_VER_POS);
  LeftAng.write(INIT_LEFT_ANG_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  RightAng.write(INIT_RIGHT_ANG_POS);

  delay(300);

  LeftAng.detach();
  RightAng.detach();
   
}

// 8
void old_mvt_surprise(void)
{

  const byte MaxAngleShift = 35;
  
  std::cout << "###old surprise" << std::endl  ;
  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);
  LeftVer.write(INIT_LEFT_VER_POS - MaxAngleShift);
  RightVer.write(INIT_RIGHT_VER_POS + MaxAngleShift);
  delay(1000);
    
  for(byte i=0; i <= MaxAngleShift; i++)
  {
    LeftVer.write(INIT_LEFT_VER_POS - MaxAngleShift + i);
    RightVer.write(INIT_RIGHT_VER_POS + MaxAngleShift - i);
    delay(30);
  }
  delay(5000);

  LeftVer.write(INIT_LEFT_VER_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  delay(500);

  LeftVer.detach();
  RightVer.detach();


}

// 9
void old_mvt_baisse(void)
{
  const byte MaxAngleShift = 30;
  unsigned int MoveDelay = 5;
  unsigned int PosDelay = 500;

  std::cout << "###old baisse" << std::endl  ;
  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);
  
  for(byte i=0; i <= MaxAngleShift; i++)
  {
    RightVer.write(INIT_LEFT_VER_POS - i);
    LeftVer.write(INIT_RIGHT_VER_POS + i);
    delay(MoveDelay);
  }
  
  for(byte i=0; i <= 3*MaxAngleShift; i++)
  {
    RightVer.write(INIT_LEFT_VER_POS - MaxAngleShift + i);
    LeftVer.write(INIT_RIGHT_VER_POS + MaxAngleShift - i);
    delay(3*MoveDelay);
  }
  
  delay(PosDelay);

  LeftVer.detach();
  RightVer.detach();

 
}

// 10
void old_mvt_tourne(void)
{
  const byte MaxAngleShift = 100;
  unsigned int MoveDelay = 10;
  unsigned int PosDelay = 500;

  
  std::cout << "###old tourne" << std::endl  ;
  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);
  
  for(byte i=0; i <= MaxAngleShift; i++)
  {
    LeftAng.write(INIT_LEFT_ANG_POS + i);
    RightAng.write(INIT_RIGHT_ANG_POS - i);
    delay(MoveDelay);
  }
  LeftAng.detach();
  RightAng.detach();
 
}

// 11
void old_mvt_reset(void)
{
  std::cout << "###old reset" << std::endl  ;

  LeftAng.attach(LeftAngPin);
  LeftVer.attach(LeftVerPin);
  RightAng.attach(RightAngPin);
  RightVer.attach(RightVerPin);

  LeftVer.write(INIT_LEFT_VER_POS);
  LeftAng.write(INIT_LEFT_ANG_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  RightAng.write(INIT_RIGHT_ANG_POS);
  delay(2000);
  
  LeftAng.detach();
  LeftVer.detach();
  RightAng.detach();
  RightVer.detach();  
}


int main(void)
{
#if 0
    std::cout << "mv_triste" << std::endl ;
    mvt_triste();
    while(!ears.step());
    old_mvt_triste();
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mvt_right" << std::endl ;
    mvt_droit();
    while(!ears.step()) {

    };
    old_mvt_droit();
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mvt_gauche" << std::endl ;
    mvt_gauche();
    while(!ears.step());
    old_mvt_gauche();
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mv_penaud" << std::endl ;
    mvt_penaud();
    while(!ears.step());
    old_mvt_penaud();
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mv_aguet" << std::endl ;
    mvt_aguet();
    while(!ears.step());
    old_mvt_aguet();
    std::cout << std::endl<< std::endl  ;
#endif
#if 0
    std::cout << "mv_reset" << std::endl ;
    mvt_reset();
    while(!ears.step());
    old_mvt_reset();
    std::cout << std::endl<< std::endl  ;
#endif
#ifdef MOVE
#define STR(a) #a
#define _MVT(a) mvt_##a
#define MVT(a) _MVT(a)
#define _OLD_MVT(a) old_mvt_##a
#define OLD_MVT(a) _OLD_MVT(a)

    std::cout << "mvt_" STR(MOVE) << std::endl ;
    MVT(MOVE)();
    while(!ears.step());
    OLD_MVT(MOVE)();
    std::cout << std::endl<< std::endl  ;
#endif
    return 0;

}
