//#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>

#define RESET_WAIT 500
#define DETACH_WAIT 500
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;

// Published values for SG90 servos; adjust if needed
int minUs = 500;
int maxUs = 2400;
                                                               /* GND ■o              ■o GND   */
                                                               /*     oo              oo 27 ???*/
// These are all GPIO pins on the ESP32                        /*     oo 26       _22_oo 25    */
// Recommended pins include 2,4,12-19,21-23,25-27,32-33·       /*     oo_18_      _21_oo 32    */
#define       LeftAltPin                     18 // blanc       /*  33 oo_19_       16 oo 12    */
#define       LeftAziPin                     19 // bleu        /*     oo 23        17 oo 04    */
#define       RightAltPin                    21 // Rouge       /*  14 oo          GND oo       */
#define       RightAziPin                    22 // noir        /*     oo 3.3V      5V oo 02    */
                                                               /*     oo 13        15 oo       */
                                                               /*     oo              oo       */
#define       INIT_LEFT_ALT              50
#define       INIT_RIGHT_ALT             85
#define       INIT_LEFT_AZI              100
#define       INIT_RIGHT_AZI             100


#define alwaysP(...) Serial.printf( __VA_ARGS__ )

//#define DEBUG_ESP_PORT Serial

#ifndef P
#ifdef DEBUG_ESP_PORT
#define P(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
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
#define Ppin(pin, str, ...) DEBUG_ESP_PORT.printf("[%lu] %s " str, millis(), pin_name(pin), ##__VA_ARGS__ )
#else
#define P(...) do{} while(0)
#define Ppin(...) do{} while(0)
#endif
#endif

//const char WiFiAPPSK[] = "oreilles"; //password wifi


template <typename T>
T sign(T val) {
    return T((val>0)-(val<0));
}


struct target {
    int dest;
    int step;
    int accel;
    unsigned int inter;
};
struct ear_target {
    struct target azi;
    struct target alt;
};
struct ears_target {
    struct ear_target left;
    struct ear_target right;
    struct ears_target *next_if_loop;
    struct ears_target *next;
    int i;
    int count;
    int wait; /* wait time before first move of this struct */
    int wait_detach; /* wait to detach a servo */
    void reset() {
        this->left.azi.dest = 0;
        this->left.azi.inter = 0;
        this->left.azi.step = 1;
        this->left.azi.accel = 0;
        this->left.alt.dest = 0;
        this->left.alt.inter = 0;
        this->left.alt.step = 1;
        this->left.alt.accel = 0;
        this->right.azi.dest = 0;
        this->right.azi.inter = 0;
        this->right.azi.step = 1;
        this->right.azi.accel = 0;
        this->right.alt.dest = 0;
        this->right.alt.inter = 0;
        this->right.alt.step = 1;
        this->right.alt.accel = 0;

        this->next = NULL;
        this->next_if_loop = NULL;

        this->i = 0;
        this->count = 1;

        this->wait = 0;
        this->wait_detach = 0;
    }

};

#define NB_MVT 6
static struct ears_target mvt_table[NB_MVT];

/////////////////////
// Wifi Definitions //
/////////////////////

//WiFiServer server(80);

class HalfEar {

    Servo _s;
    int _pin;
    int _neuter;
    int _way;
    int _cur;
    bool _finish;
    int _dest;
    int _step;
    int _accel;
    unsigned int _inter;
    unsigned long _wait;
    unsigned long _wait_detach;

    void _moveto(int offset, unsigned long now) {
        this->_cur = offset;
        this->_wait = now + this->_inter;
        if (!this->_s.attached()) {
            /* ERROR */
            Ppin(this->_pin, "SHOULD BE ATTACHED\n");
        }
        //P("***");
        Ppin(this->_pin, "write %d angle (%d)\n",offset + this->_neuter, offset);
        this->_s.write(offset + this->_neuter);
    }

public:
    HalfEar(int pin, int neuter, int way) : _pin(pin), _neuter(neuter), _way(sign(way)), _cur(-100), _finish(false) {
         _s.setPeriodHertz(50); // Standard 50hz servo
    }

    void attach() {
        Ppin(this->_pin, "Attach\n");
        this->_s.attach(this->_pin, minUs, maxUs);
    }
    void detach() {
        Ppin(this->_pin, "Detach\n");
        this->_s.detach();
    }

    void moveto(int offset, unsigned long now) {
        Ppin(this->_pin, "half ear move to offset %d\n", offset);
        this->_moveto(offset, now);
    }
    bool step(unsigned long now) {
        // is it time ?
        if (!this->_s.attached()) {
            Ppin(this->_pin, "not attached, nothing to do, move ended\n");
            return true;
        } else {
            Ppin(this->_pin, "is it time now=%lu, wait=%lu\n",now, this->_wait);
            if (now >= this->_wait) {
                if (this->_finish) {
                    Ppin(this->_pin, "now detach\n");
                    this->_s.detach();
                    this->_finish = false;
                    return true;
                }
                Ppin(this->_pin, "now move dest=%d, cur=%d, step=%d\n", this->_dest, this->_cur, this->_step);
                if (this->_inter == 0) {
                    this->moveto(this->_dest, now);
                    // end of move for this servo
                    this->_wait = now + this->_wait_detach;
                    this->_finish = true;
                } else {
                    int prev_pos = this->_cur;
                    this->moveto(this->_cur + this->_step, now);
                    this->_step += this->_accel;
                    if ((this->_dest == this->_cur) // dest reached
                       or (sign(this->_dest - prev_pos) != sign(this->_dest - this->_cur))){ // dest overtaken
                        // end of move for this servo
                        this->_wait = now + this->_wait_detach;
                        this->_finish = true;
                    }
                }
            }
            else {Ppin(this->_pin, "not time to move\n"); }
            return false;
        }
    }

    void define_move(struct target *move, unsigned long at, unsigned long wait_detach) {
        this->_dest = this->_way * move->dest;
        this->_inter = move->inter ;
        this->_step = move->step ? move->step : 1;
        this->_accel = move->accel;
        if( sign(this->_dest - this->_cur) != sign(this->_step)) {
            /* got to right way */
            this->_step = -this->_step;
            this->_accel = -this->_accel;
        }
        this->_finish = false;
        this->_wait = at;
        this->_wait_detach = wait_detach;
        P("***");
        Ppin(this->_pin, "define move half-ear inter=%u dest=%d, cur=%d\n", this->_inter, this->_dest, this->_cur);
    }
};

class Ear {
    HalfEar _alt;
    HalfEar _azi;
    public:
    Ear(int altPin, int altNeuter, int aziPin, int aziNeuter, int alt_way) : _alt(altPin, altNeuter, alt_way),
                                                                _azi(aziPin, aziNeuter, 1) {}

    void attach() {
        this->_azi.attach();
        this->_alt.attach();
    }
    void detach() {
        this->_azi.detach();
        this->_alt.detach();
    }

    bool step(unsigned long now) {
        bool finish;
        finish = this->_alt.step(now);
        finish = this->_azi.step(now) and finish ;
        return finish;
    }

    void define_move(struct ear_target *move, unsigned long at, unsigned long wait_detach) {
        this->_alt.define_move(&move->alt, at, wait_detach);
        this->_azi.define_move(&move->azi, at, wait_detach);
    }
};

struct ears {
    Ear left;
    Ear right;
    struct ears_target *move;

    ears(int lAltPin, int lAltNeuter,
         int lAziPin, int lAziNeuter,
         int rAltPin, int rAltNeuter,
         int rAziPin, int rAziNeuter) :
                left(lAltPin, lAltNeuter, lAziPin, lAziNeuter, -1),
                right(rAltPin, rAltNeuter, rAziPin, rAziNeuter, 1) {}

    bool step(unsigned long now) {
        bool finish;
        if(move) {
            finish = this->left.step(now);
            finish = this->right.step(now) and finish ;
            if(finish) {
                P("finish move i=%d, count=%d\n", move->i, move->count);
                if (--(move->i) <= 0) {
                    P("next move %p\n\n", move->next);
                    this->define_move(move->next, now);
                } else {
                    P("next loop move %p\n\n", move->next_if_loop);
                    this->define_move(move->next_if_loop, now);
                }

            }
            return false;
        } else {
            return true;
        }
    }
    void define_move(struct ears_target* m, unsigned long now) {
        this->move = m;
        if (m) {
            if (m->i <= 0)
                m->i = m->count;
            this->left.attach();
            this->left.define_move(&m->left, m->wait + now, m->wait_detach);
            this->right.attach();
            this->right.define_move(&m->right, m->wait + now, m->wait_detach);
        } else {
            this->left.detach();
            this->right.detach();
        }
    }
} ears(LeftAltPin, INIT_LEFT_ALT,
       LeftAziPin, INIT_LEFT_AZI,
       RightAltPin, INIT_RIGHT_ALT,
       RightAziPin, INIT_RIGHT_AZI) ;

bool incomming_msg = false;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      incomming_msg = true;
    }
};


void setup()
{
#if 0
    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    #define WL_MAC_ADDR_LENGTH 6
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
        String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "Neko_Mimi " + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i=0; i<AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);


    server.begin();
#endif

    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    BLEDevice::init("CatEars");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,
                                            BLECharacteristic::PROPERTY_READ |
                                            BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Started");
    pService->start();
    incomming_msg = false;
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();

    for (int i =0 ; i < NB_MVT ; i++) {
        mvt_table[i].reset();
        mvt_table[i].wait_detach = 500;
    }
    ears.define_move(&mvt_table[0], millis());

    P("setup done\n");
}

//1
void mvt_triste(unsigned long now) /* Ok for pos, check time */
{
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].right.alt.dest = -35; /**/
    mvt_table[0].left.alt.dest = -35; /**/
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].left.alt.dest = -65; /**/
    mvt_table[1].left.alt.inter = 5;
    mvt_table[1].left.alt.step = 1;
    mvt_table[1].right.alt.dest = -65; /**/
    mvt_table[1].right.alt.inter = 5;
    mvt_table[1].right.alt.step = 1;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].left.alt.dest = 25; /**/
    mvt_table[2].left.alt.inter = 5;
    mvt_table[2].left.alt.step = 1;
    mvt_table[2].right.alt.dest = 25; /**/
    mvt_table[2].right.alt.inter = 5;
    mvt_table[2].right.alt.step = 1;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].wait = RESET_WAIT;
    mvt_table[3].wait_detach = DETACH_WAIT;

    ears.define_move(&mvt_table[0], now);
}

//2
void mvt_penaud(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].left.azi.dest = 100; /**/
    mvt_table[1].left.azi.inter = 10;
    mvt_table[1].left.azi.step = 1;
    mvt_table[1].left.alt.dest = -35; /**/
    mvt_table[1].right.azi.dest = -100;/**/
    mvt_table[1].right.azi.inter = 10;
    mvt_table[1].right.azi.step = 1;
    mvt_table[1].right.alt.dest = -35;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].left.azi.dest = 100;/**/
    mvt_table[2].left.alt.dest = -55;/**/
    mvt_table[2].left.alt.inter = 10;
    mvt_table[2].left.alt.step = 1;
    mvt_table[2].right.azi.dest = -100;/**/
    mvt_table[2].right.alt.dest = -50;
    mvt_table[2].right.alt.inter = 10;
    mvt_table[2].right.alt.step = 1;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].left.azi.dest = 100;/* */
    mvt_table[3].left.alt.dest = 5;
    mvt_table[3].left.alt.inter = 30;
    mvt_table[3].left.alt.step = 1;
    mvt_table[3].right.azi.dest = -100;/**/
    mvt_table[3].right.alt.dest = 0;
    mvt_table[3].right.alt.inter = 30;
    mvt_table[3].right.alt.step = 1;
    mvt_table[3].next =  &mvt_table[4];

    mvt_table[4].reset();
    mvt_table[4].wait = RESET_WAIT ;
    mvt_table[4].wait_detach = DETACH_WAIT;

    ears.define_move(&mvt_table[0], now);

}

//3
void mvt_gauche(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].left.azi.dest = 50;
    mvt_table[1].left.azi.inter = 3;
    mvt_table[1].left.azi.step = 1;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].left.azi.dest = -50;
    mvt_table[2].left.alt.dest = -50;
    mvt_table[2].left.alt.inter = 10;
    mvt_table[2].left.alt.step = 1;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].left.azi.dest = -50;
    mvt_table[3].left.alt.dest = 70;
    mvt_table[3].left.alt.inter = 10;
    mvt_table[3].left.alt.step = 1;
    mvt_table[3].left.alt.accel = 1;
    mvt_table[3].next =  &mvt_table[4];

    mvt_table[4].reset();
    mvt_table[4].left.azi.dest = -50;
    mvt_table[4].left.alt.dest = -50;
    mvt_table[4].left.alt.inter = 10;
    mvt_table[4].left.alt.step = 1;
    mvt_table[4].left.alt.accel = 1;
    mvt_table[4].next =  &mvt_table[5];
    mvt_table[4].count = 6;
    mvt_table[4].next_if_loop = &mvt_table[3];
    mvt_table[4].next =  &mvt_table[5];

    mvt_table[5].reset();
    mvt_table[5].wait = RESET_WAIT;
    mvt_table[5].wait_detach = DETACH_WAIT;

    ears.define_move(&mvt_table[0], now);
}

//4
void mvt_droit(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].right.azi.dest = -50;
    mvt_table[1].right.azi.inter = 3;
    mvt_table[1].right.azi.step = 1;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].right.azi.dest = -50;
    mvt_table[2].right.alt.dest = -50;
    mvt_table[2].right.alt.inter = 10;
    mvt_table[2].right.alt.step = 1;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].right.azi.dest = -50;
    mvt_table[3].right.alt.dest = 70;
    mvt_table[3].right.alt.inter = 10;
    mvt_table[3].right.alt.step = 1;
    mvt_table[3].right.alt.accel = 1;
    mvt_table[3].next =  &mvt_table[4];

    mvt_table[4].reset();
    mvt_table[4].right.azi.dest = -50;
    mvt_table[4].right.alt.dest = -50;
    mvt_table[4].right.alt.inter = 10;
    mvt_table[4].right.alt.step = 1;
    mvt_table[4].right.alt.accel = 1;
    mvt_table[4].next =  &mvt_table[5];
    mvt_table[4].count = 6;
    mvt_table[4].next_if_loop = &mvt_table[3];
    mvt_table[4].next =  &mvt_table[5];

    mvt_table[5].reset();
    mvt_table[5].wait = RESET_WAIT;
    mvt_table[5].wait_detach = DETACH_WAIT;

    ears.define_move(&mvt_table[0], now);
}

//5
void mvt_aguet(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].right.azi.dest = -50;
    mvt_table[1].right.azi.inter = 3;
    mvt_table[1].right.azi.step = 1;
    mvt_table[1].left.azi.dest = 50;
    mvt_table[1].left.azi.inter = 3;
    mvt_table[1].left.azi.step = 1;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].right.azi.dest = -50;
    mvt_table[2].left.azi.dest = 50;
    mvt_table[2].right.alt.dest = -50;
    mvt_table[2].right.alt.inter = 3;
    mvt_table[2].right.alt.step = 1;
    mvt_table[2].left.alt.dest = -50;
    mvt_table[2].left.alt.inter = 3;
    mvt_table[2].left.alt.step = 1;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].right.azi.dest = -50;
    mvt_table[3].left.azi.dest = 50;
    mvt_table[3].right.alt.dest = 5;
    mvt_table[3].right.alt.inter = 10;
    mvt_table[3].right.alt.step = 1;
    mvt_table[3].right.alt.accel = 1;
    mvt_table[3].left.alt.dest = 5;
    mvt_table[3].left.alt.inter = 10;
    mvt_table[3].left.alt.step = 1;
    mvt_table[3].left.alt.accel = 1;
    mvt_table[3].next =  &mvt_table[4];

    mvt_table[4].reset();
    mvt_table[4].right.azi.dest = -50;
    mvt_table[4].left.azi.dest = 50;
    mvt_table[4].right.alt.dest = -50;
    mvt_table[4].right.alt.inter = 10;
    mvt_table[4].right.alt.step = 1;
    mvt_table[4].right.alt.accel = 1;
    mvt_table[4].left.alt.dest = -50;
    mvt_table[4].left.alt.inter = 10;
    mvt_table[4].left.alt.step = 1;
    mvt_table[4].left.alt.accel = 1;
    mvt_table[4].next =  &mvt_table[5];
    mvt_table[4].count = 6;
    mvt_table[4].next_if_loop = &mvt_table[3];
    mvt_table[4].next =  &mvt_table[5];

    mvt_table[5].reset();
    mvt_table[5].wait = RESET_WAIT;
    mvt_table[5].wait_detach = DETACH_WAIT;

    ears.define_move(&mvt_table[0], now);

}

//6
void mvt_content(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].right.alt.dest = -50;
    mvt_table[0].left.alt.dest = 50;
    mvt_table[0].wait = 300;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].right.alt.dest = 50;
    mvt_table[1].left.alt.dest = -50;
    mvt_table[1].wait = 300;
    mvt_table[1].count = 3;
    mvt_table[1].next_if_loop = &mvt_table[0];
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].wait = RESET_WAIT;
    mvt_table[2].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);
}

// 7
void mvt_ecoute(unsigned long now)
{
    /* not yet checked */
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].right.azi.dest = -100;
    mvt_table[1].right.azi.inter = 10;
    mvt_table[1].right.azi.step = 1;
    mvt_table[1].right.azi.accel = 1;
    mvt_table[1].left.azi.dest = 100;
    mvt_table[1].left.azi.inter = 10;
    mvt_table[1].left.azi.step = 1;
    mvt_table[1].left.azi.accel = 1;
    mvt_table[1].next =  &mvt_table[2];


    mvt_table[2].reset();
    mvt_table[2].wait = 300;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].wait = 300;
    mvt_table[3].right.azi.dest = 100;
    mvt_table[3].right.azi.inter = 10;
    mvt_table[3].right.azi.step = 1;
    mvt_table[3].right.azi.accel = 1;
    mvt_table[3].left.azi.dest = -100;
    mvt_table[3].left.azi.inter = 10;
    mvt_table[3].left.azi.step = 1;
    mvt_table[3].left.azi.accel = 1;
    mvt_table[3].next =  &mvt_table[4];

    mvt_table[4].reset();
    mvt_table[4].wait = 300;
    mvt_table[4].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);
}

// 8
void mvt_surprise(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].right.alt.dest = 35;
    mvt_table[0].left.alt.dest = 35;
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].wait = 1000;
    mvt_table[1].right.alt.dest = 0;
    mvt_table[1].right.alt.inter = 30;
    mvt_table[1].right.alt.step = 1;
    mvt_table[1].right.alt.accel = 1;
    mvt_table[1].left.alt.dest = 0;
    mvt_table[1].left.alt.inter = 30;
    mvt_table[1].left.alt.step = 1;
    mvt_table[1].left.alt.accel = 1;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].wait = RESET_WAIT;
    mvt_table[2].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);
}

// 9
void mvt_baisse(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait = RESET_WAIT;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].right.alt.dest = -30;
    mvt_table[1].right.alt.inter = 15;
    mvt_table[1].right.alt.step = 1;
    mvt_table[1].right.alt.accel = 1;
    mvt_table[1].left.alt.dest = -30;
    mvt_table[1].left.alt.inter = 15;
    mvt_table[1].left.alt.step = 1;
    mvt_table[1].left.alt.accel = 1;
    mvt_table[1].next =  &mvt_table[2];

    mvt_table[2].reset();
    mvt_table[2].right.alt.dest = 90;
    mvt_table[2].right.alt.inter = 15;
    mvt_table[2].right.alt.step = 1;
    mvt_table[2].right.alt.accel = 1;
    mvt_table[2].left.alt.dest = 90;
    mvt_table[2].left.alt.inter = 15;
    mvt_table[2].left.alt.step = 1;
    mvt_table[2].left.alt.accel = 1;
    mvt_table[2].next =  &mvt_table[3];

    mvt_table[3].reset();
    mvt_table[3].wait = RESET_WAIT;
    mvt_table[3].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);
}

// 10
void mvt_tourne(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait = 10;
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].reset();
    mvt_table[1].right.azi.dest = 100;
    mvt_table[1].right.azi.inter = 15;
    mvt_table[1].right.azi.step = 1;
    mvt_table[1].right.azi.accel = 1;
    mvt_table[1].left.azi.dest = 100;
    mvt_table[1].left.azi.inter = 15;
    mvt_table[1].left.azi.step = 1;
    mvt_table[1].left.azi.accel = 1;
    mvt_table[1].next =  &mvt_table[2];


    mvt_table[2].reset();
    mvt_table[2].right.azi.dest = -100;
    mvt_table[2].right.azi.inter = 10;
    mvt_table[2].right.azi.step = 1;
    mvt_table[2].right.azi.accel = 1;
    mvt_table[2].left.azi.dest = -100;
    mvt_table[2].left.azi.inter = 10;
    mvt_table[2].left.azi.step = 1;
    mvt_table[2].left.azi.accel = 1;
    mvt_table[2].next =  &mvt_table[2];



    mvt_table[3].reset();
    mvt_table[3].wait = 10;
    mvt_table[3].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);


}

// 11
void mvt_reset(unsigned long now)
{
    mvt_table[0].reset();
    mvt_table[0].wait_detach = 500;
    mvt_table[0].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);
}

// manual
void mvt_manual(unsigned long now, int coord[4])
{
    mvt_table[0].reset();
    mvt_table[0].right.azi.dest = coord[0];
    mvt_table[0].right.azi.inter = 10;
    mvt_table[0].right.azi.step = 1;
    mvt_table[0].right.alt.dest = coord[1];
    mvt_table[0].right.alt.inter = 10;
    mvt_table[0].right.alt.step = 1;
    mvt_table[0].left.azi.dest = coord[2];
    mvt_table[0].left.azi.inter = 10;
    mvt_table[0].left.azi.step = 1;
    mvt_table[0].left.alt.dest = coord[3];
    mvt_table[0].left.alt.inter = 10;
    mvt_table[0].left.alt.step = 1;
    mvt_table[0].count = 3;
    mvt_table[0].next_if_loop = &mvt_table[0];
    mvt_table[0].next =  NULL ;


    mvt_table[0].wait_detach = DETACH_WAIT;
    ears.define_move(&mvt_table[0], now);


}

void loop()
{
  unsigned long now;

  now = millis();
  ears.step(now);

  if(incomming_msg) {
    std::string value = pCharacteristic->getValue();
    incomming_msg = false;

    alwaysP("manage %s\n", value.c_str());
    if (value == "triste") {
        mvt_triste(now);
    } else if (value == "penaud") {
        mvt_penaud(now);
    } else if (value == "gauche") {
        mvt_gauche(now);
    } else if (value == "droit") {
        mvt_droit(now);
    } else if (value == "aguet") {
        mvt_aguet(now);
    } else if (value == "content") {
        mvt_content(now);
    } else if (value == "ecoute") {
        mvt_ecoute(now);
    } else if (value == "surprise") {
        mvt_surprise(now);
    } else if (value == "baisse") {
        mvt_baisse(now);
    } else if (value == "tourne") {
        mvt_tourne(now);
    } else if (value == "reset") {
        mvt_reset(now);
    }
  }
}
