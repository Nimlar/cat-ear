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
    }

};

#define NB_MVT 6
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
    int _step;
    int _accel;
    unsigned int _inter;
    unsigned long _wait;

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
            Ppin(this->_pin, "is it time now=%lu, wait=%lu\n",now, this->_wait);
            if (now >= this->_wait) {
                Ppin(this->_pin, "now move dest=%d, cur=%d, step=%d\n", this->_dest, this->_cur, this->_step);
                if (this->_inter == 0) {
                    this->moveto(this->_dest);
                    // end of move for this servo
                    this->_s.detach();
                    return true;
                } else {
                    int prev_pos = this->_cur;
                    this->moveto(this->_cur + this->_step);
                    this->_step += this->_accel;
                    if ((this->_dest == this->_cur) // dest reached
                       or (sign(this->_dest - prev_pos) != sign(this->_dest - this->_cur))){ // dest overtaken
                        // end of move for this servo
                        this->_s.detach();
                        return true;

                    }
                }
            }
            else {Ppin(this->_pin, "not time to move\n"); }
            return false;
        }
    }

    void define_move(struct target *move, int wait) {
        this->_dest = this->_way * move->dest;
        this->_inter = move->inter * 10;
        this->_step = move->step ? move->step : 1;
        this->_accel = move->accel;
        if( sign(this->_dest - this->_cur) != sign(this->_step)) {
            /* got to right way */
            this->_step = -this->_step;
            this->_accel = -this->_accel;
        }
        this->_wait = millis() + wait;
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

    void define_move(struct ear_target *move, int wait) {
        this->_alt.define_move(&move->alt, wait);
        this->_azi.define_move(&move->azi, wait);
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
            this->left.define_move(&m->left, m->wait);
            this->right.attach();
            this->right.define_move(&m->right, m->wait);
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
void mvt_triste(void) /* Ok for pos, check time */
{
    mvt_table[0].reset();
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

    ears.define_move(&mvt_table[0]);
}

//2
void mvt_penaud(void) /* XXX */
{
    mvt_table[0].reset();
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
    mvt_table[3].left.alt.inter = 10;
    mvt_table[3].left.alt.step = 1;
    mvt_table[3].right.azi.dest = -100;/**/
    mvt_table[3].right.alt.dest = 0;
    mvt_table[3].right.alt.inter = 30;
    mvt_table[3].right.alt.step = 1;
    mvt_table[3].next =  &mvt_table[4];

    mvt_table[4].reset();
    mvt_table[4].wait = 300 ;

    ears.define_move(&mvt_table[0]);

}

//3
void mvt_gauche(void)
{
    mvt_table[0].reset();
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

    ears.define_move(&mvt_table[0]);
}

//4
void mvt_droit(void)
{
    mvt_table[0].reset();
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

    ears.define_move(&mvt_table[0]);
}

//5
void mvt_aguet(void)
{
    mvt_table[0].reset();
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

    ears.define_move(&mvt_table[0]);

}

//6
void mvt_content(void)
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
    ears.define_move(&mvt_table[0]);


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

