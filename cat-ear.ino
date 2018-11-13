//arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
#ifndef PC_EMUL
#include <ESP8266WiFi.h>
#include <Servo.h>
#endif

//////////////////////
// Pins Definitions //
//////////////////////
#define       LeftAltPin                     0  //D3 blanc
#define       LeftAziPin                     2  //D4 bleu
#define       RightAltPin                    16 //D0 Rouge
#define       RightAziPin                    5  //D1 noir


#define       INIT_LEFT_ALT              50
#define       INIT_RIGHT_ALT             85
#define       INIT_LEFT_AZI              100
#define       INIT_RIGHT_AZI             100


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
#define Ppin(pin, str, ...) DEBUG_ESP_PORT.printf("%s " str, pin_name(pin), ##__VA_ARGS__ )
#else
#define P(...) do{} while(0)
#define Ppin(...) do{} while(0)
#endif
#endif

static const char *http_answer=" HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
static const char *html_answer=R"rawHTML(
<html>
<head>
<meta charset="utf-8">
<title>Neko_Mimi</title>
<style>
html, body {
  background: -webkit-linear-gradient(left, #e0e3ea 50%, #e3e6ec 50%);
  background: -moz-linear-gradient(left, #e0e3ea 50%, #e3e6ec 50%);
  background: linear-gradient(left, #e0e3ea 50%, #e3e6ec 50%);
  background-size: 10px 10px;
}
#main {
  margin-bottom: 2em;
  vertical-align:middle;
}
.page-header {
  border-bottom-color: #b0b4bc;
  font-size: 200%;
  text-align:center;
}
.ios-dl .legend {
  display: block;
  padding: 0;
  margin: 20px 0px 0px;
  font-size: 21px;
  line-height: 40px;
  color: #8B91A5;
  font-weight: bold;
}
.ios-dl .definition-group {
  background: #fff;
  border-radius: 10px;
  border: 1px solid #b0b4bc;
  box-shadow: 0 1px 1px 0 white, 0 1px 1px 0 #d9dbdf inset;
}
.ios-dl dl {
  padding: 1em;
  margin: 0px;
  background: transparent;
  border-top: 1px solid #d9dbdf;
}
.ios-dl dl:first-child {
  border: none;
}
.ios-dl dt {
  text-align: center;
  font-size: 250%;
  font-weight: bold;
}
a:hover, a:visited, a:link, a:active
{
  text-decoration: none;
  display:block;width:100%;height:100%;
  color:  #A0A0A0;
  font-size: 150%;
}
img {
  vertical-align:middle;
}
</style>
</head>

<body>
<div id="main" class="container-fluid">
  <div class="page-header">
  </div>
  <div class="row-fluid">
    <div class="span12">
      <div class="main ios-dl">
        <span class="legend"></span>
        <div class="definition-group">
          <dl class="dl-horizontal">
            <dt><a href="/mvt1/">Triste</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt2/">Penaud</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt3/">Oreille Gauche</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt4/">Oreille Droite</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt5/">Aux aguets</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt6/">Content</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt7/">écoute</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt8/">Surprise</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt9/">Baissées</a></dt>
          </dl>
          <dl class="dl-horizontal">
            <dt><a href="/mvt10/">Tournées</a></dt>
          </dl>
         <dl class="dl-horizontal">
            <dt><a href="/mvt11/">Reset Origine</a></dt>
          </dl>
        </div>
      </div>
    </div>
  </div>
</body>
</html>

)rawHTML";
const char WiFiAPPSK[] = "oreilles"; //password wifi


template <typename T>
T sign(T val) {
    return T((val>0)-(val<0));
}


struct target {
    int dest;
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

    void set(struct ear_target l, struct ear_target r, struct ears_target *n) {
        this->i = 0;
        this->count = 1;
        this->next = n;
        this->next_if_loop = NULL;
        this->left = l;
        this->right = r;
    }

    void reset() {
        this->set({} , {}, NULL);
    }

};

#define NB_MVT 5
static struct ears_target mvt_table[NB_MVT];

/////////////////////
// Wifi Definitions //
/////////////////////

WiFiServer server(80);

class HalfEar {

    Servo _s;
    int _pin;
    int _neuter;
    int _way;
    int _cur;
    int _dest;
    unsigned int _inter;
    unsigned long _last;

    void _moveto(int offset, unsigned long now) {
        this->_cur = offset;
        this->_last = now;
        if (!this->_s.attached()) {
            /* ERROR */
            Ppin(this->_pin, "SHOULD BE ATTACHED\n");
        }
        P("***");
        Ppin(this->_pin, "send angle = %d\n", offset + this->_neuter);
        this->_s.write(offset + this->_neuter);
    }

public:
    HalfEar(int pin, int neuter, int way) : _pin(pin), _neuter(neuter), _way(sign(way)), _cur(-100) {}

    void attach() {
        Ppin(this->_pin, "Attach\n");
        this->_s.attach(this->_pin);
    }
    void detach() {
        Ppin(this->_pin, "Detach\n");
        this->_s.detach();
    }

    /*void moveto(int angle, unsigned long now) {
        this->_moveto(angle, now);
    }*/
    void moveto(int offset) {
        Ppin(this->_pin, "half ear move to offset %d\n", offset);
        this->_moveto(offset, millis());
    }
    bool step(unsigned long now) {
        // is it time ?
        if (!this->_s.attached()) {
            Ppin(this->_pin, "not attached, nothing to do, move ended\n");
            return true;
        } else {
            Ppin(this->_pin, "is it time now=%lu, last=%lu, inter=%u\n",now, this->_last, this->_inter);
            if ((this->_last + this->_inter) <= now) {
                int incr = sign(this->_dest - this->_cur);
                Ppin(this->_pin, "now move dest=%d, cur=%d, incr=%d\n", this->_dest, this->_cur, incr);
                if (!incr) {
                    // end of move for this servo
                    this->_s.detach();
                    return true;
                }
                if (this->_inter == 0) {
                    this->moveto(this->_dest);
                } else {
                    this->moveto(this->_cur + incr);
                }
            }
            else {Ppin(this->_pin, "not time to move\n"); }
            return false;
        }
    }

    void define_move(struct target *move) {
        this->_dest = this->_way * move->dest;
        this->_inter = move->inter * 10;
        P("***");
        Ppin(this->_pin, "define move half-ear inter=%u dest=%d, cur=%d\n", this->_inter, this->_dest, this->_cur);
    }
};

class Ear {
    HalfEar _alt;
    HalfEar _azi;
    public:
    Ear(int altPin, int altNeuter, int aziPin, int aziNeuter, int azi_way) : _alt(altPin, altNeuter, 1),
                                                                _azi(aziPin, aziNeuter, azi_way) {}

    void attach() {
        this->_azi.attach();
        this->_alt.attach();
    }
    void detach() {
        this->_azi.detach();
        this->_alt.detach();
    }


    void moveto(int aziOffset, int altOffset) {
        this->_azi.moveto(aziOffset);
        this->_alt.moveto(altOffset);
    }

    bool step(unsigned long now) {
        bool finish;
        finish = this->_alt.step(now);
        finish = this->_azi.step(now) and finish ;
        return finish;
    }

    void define_move(struct ear_target *move) {
        this->_alt.define_move(&move->alt);
        this->_azi.define_move(&move->azi);
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
                left(lAltPin, lAltNeuter, lAziPin, lAziNeuter, 1),
                right(rAltPin, rAltNeuter, rAziPin, rAziNeuter, -1) {}

    bool step() {
        bool finish;
        if(move) {
            unsigned long now = millis();
            finish = this->left.step(now);
            finish = this->right.step(now) and finish ;
            if(finish) {
                P("finish move i=%d, count=%d\n", move->i, move->count);
                if (--(move->i) <= 0) {
                    P("next move %p\n\n", move->next);
                    this->define_move(move->next);
                } else {
                    P("next loop move %p\n\n", move->next_if_loop);
                    this->define_move(move->next_if_loop);
                }

            }
            return false;
        } else {
            return true;
        }
    }
    void define_move(struct ears_target* m) {
        this->move = m;
        if (m) {
            if (m->i <= 0)
                m->i = m->count;
            this->left.attach();
            this->left.define_move(&m->left);
            this->right.attach();
            this->right.define_move(&m->right);
        } else {
            this->left.detach();
            this->right.detach();
        }
    }
} ears(LeftAltPin, INIT_LEFT_ALT,
       LeftAziPin, INIT_LEFT_AZI,
       RightAltPin, INIT_RIGHT_ALT,
       RightAziPin, INIT_RIGHT_AZI) ;

void setup()
{
#ifndef PC_EMUL
    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
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

#ifdef DEBUG_ESP_PORT
    Serial.begin(115200);
#endif

    server.begin();
#endif

    mvt_table[0].reset();
    ears.define_move(&mvt_table[0]);
    
    P("setup done\n");
}

//1
void mvt_triste(void)
{
    mvt_table[0].reset();
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].set({ .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 30, .inter = 5 }},
                     { .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 30, .inter = 5 }},
                     &mvt_table[2]);

    mvt_table[2].reset();

    ears.define_move(&mvt_table[0]);
}

//2
void mvt_penaud(void)
{
    mvt_table[0].reset();
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].set({ .azi = { .dest =100, .inter = 10 }, .alt = { .dest = 0, .inter = 0 }},
                     { .azi = { .dest = -100, .inter = 10 }, .alt = { .dest = 0, .inter = 0 }},
                     &mvt_table[2]);

    mvt_table[2].set({ .azi = { .dest = 100, .inter = 0 }, .alt = { .dest = -20, .inter = 10 }},
                     { .azi = { .dest = -100, .inter = 0 },.alt = { .dest = +20, .inter = 10 }},
                     &mvt_table[3]);


    mvt_table[3].set({ .azi = { .dest = +100, .inter = 0 }, .alt = { .dest = 0, .inter = 30 }},
                     { .azi = { .dest = -100, .inter = 0 }, .alt = { .dest = 0, .inter = 30 }},
                     &mvt_table[4]);

    mvt_table[4].reset();

    ears.define_move(&mvt_table[0]);

}

//3
void mvt_gauche(void)
{
    mvt_table[0].reset();
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].set({ .azi = { .dest =50, .inter = 3 }, .alt = { .dest =50, .inter = 3 }},
                     { .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 0, .inter = 0 }},
                     &mvt_table[2]);
    mvt_table[2].set({ .azi = { .dest = 50, .inter = 0 }, .alt = { .dest = 35, .inter = 10 }},
                     { .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 0, .inter = 0 }},
                     &mvt_table[3]);

    mvt_table[3].set({ .azi = { .dest = 50, .inter =  0 }, .alt = { .dest = 50, .inter = 10 }},
                     { .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 0, .inter = 0 }},
                     &mvt_table[4]);
    mvt_table[3].count = 5;
    mvt_table[3].next_if_loop = &mvt_table[2];

    mvt_table[4].reset();

    ears.define_move(&mvt_table[0]);
}

//4
void mvt_droit(void)
{
    mvt_table[0].reset();
    mvt_table[0].next = &mvt_table[1];

    mvt_table[1].set({ .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 0, .inter = 0 }},
                     { .azi = { .dest =50, .inter = 3 }, .alt = { .dest =50, .inter = 3 }},
                     &mvt_table[2]);
    mvt_table[2].set({ .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 0, .inter = 0 }},
                     { .azi = { .dest = 50, .inter = 10 }, .alt = { .dest = 35, .inter = 10 }},
                     &mvt_table[3]);

    mvt_table[3].set({ .azi = { .dest = 0, .inter = 0 }, .alt = { .dest = 0, .inter = 0 }},
                     { .azi = { .dest = 50, .inter = 10 }, .alt = { .dest = 50, .inter = 10 }},
                     &mvt_table[4]);
    mvt_table[3].count = 5;
    mvt_table[3].next_if_loop = &mvt_table[2];

    mvt_table[4].reset();

    ears.define_move(&mvt_table[0]);
}

//5
void mvt_aguet(void)
{
}

//6
void mvt_content(void)
{

}

// 7
void mvt_ecoute(void)
{
}

// 8
void mvt_surprise(void)
{

}

// 9
void mvt_baisse(void)
{
}

// 10
void mvt_tourne(void)
{
}

// 11
void mvt_reset(void)
{
    mvt_table[0].reset();
    ears.define_move(&mvt_table[0]);
}

void loop()
{
  ears.step();

#ifndef PC_EMUL
// Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

    // Read the first line of the request
  String request = client.readStringUntil('\r');
  client.flush();



  // Match the request
  if (request.indexOf("/mvt1/") != -1)  {
    mvt_triste();
  } else if (request.indexOf("/mvt2/") != -1)  {
    mvt_penaud();
  } else if (request.indexOf("/mvt3/") != -1)  {
    mvt_gauche();
  } else if (request.indexOf("/mvt4/") != -1)  {
    mvt_droit();
  } else if (request.indexOf("/mvt5/") != -1)  {
    mvt_aguet();
  } else if (request.indexOf("/mvt6/") != -1)  {
    mvt_content();
  } else if (request.indexOf("/mvt7/") != -1)  {
    mvt_ecoute();
  } else if (request.indexOf("/mvt8/") != -1)  {
    mvt_surprise();
  } else if (request.indexOf("/mvt9/") != -1)  {
    mvt_baisse();
  } else if (request.indexOf("/mvt10/") != -1)  {
    mvt_tourne();
  } else if (request.indexOf("/mvt11/") != -1)  {
    mvt_reset();
  }

  // Return the response
  client.print(http_answer);
  client.print(html_answer);
#endif
  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

