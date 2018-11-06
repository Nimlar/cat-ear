//arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .

#include <ESP8266WiFi.h>
#include <Servo.h>


const char *http_answer=" HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
const char *html_answer=R"rawHTML(
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
//////////////////////
// Pins Definitions //
//////////////////////

const char WiFiAPPSK[] = "oreilles"; //password wifi

#define       LeftAltPin                     0  //D3 blanc
#define       LeftAziPin                     2  //D4 bleu
#define       RightAltPin                    16 //D0 Rouge
#define       RightAziPin                    5  //D1 noir


#define       INIT_LEFT_ALT              50
#define       INIT_RIGHT_ALT             85
#define       INIT_LEFT_AZI              100
#define       INIT_RIGHT_AZI             100

template <typename T>
T sign(T val) {
    return T((val>0)-(val<0));
}


struct target {
    int dest;
    int inter;
};
struct ear_target {
    struct target azi;
    struct target alt;
};
struct ears_target {
    struct ear_target left;
    struct ear_target right;
    struct ears_target *next;
};

#define NB_MVT 5
struct ears_target mvt_table[NB_MVT];

/////////////////////
// Wifi Definitions //
/////////////////////

WiFiServer server(80);

class HalfEar {

    Servo _s;
    int _dest;
    int _cur;
    int _inter;
    int _neuter;
    unsigned long _last;
    int _pin;

    void _moveto(int offset, unsigned long now) {
        this->_cur = offset;
        this->_last = now;
        if (!this->_s.attached()) {
            /* ERROR */
        }
        this->_s.write(offset + this->_neuter);
    }

    public:
    HalfEar(int pin, int neuter) : _pin(pin), _neuter(neuter) {};

    void attach() {
        this->_s.attach(this->_pin);
    }
    void detach() {
        this->_s.detach();
    }

    /*void moveto(int angle, unsigned long now) {
        this->_moveto(angle, now);
    }*/
    void moveto(int offset) {
        this->_moveto(offset, millis());
    }
    bool step(unsigned long now) {
        // is it time ?
        if (this->_last + this->_inter <= now) {
            int incr = sign(this->_dest - this->_cur);
            if (!incr) {
                // end of move for this servo
                this->_s.detach();
                return true;
            }
            if (this->_inter == -1) {
                this->moveto(this->_dest);
            } else {
                this->moveto(this->_cur + incr);
            }
        }
        return false;
    }

    void define_move(struct target *move) {
        this->_dest = move->dest;
        this->_inter = move->inter;
    }
};

class Ear {
    HalfEar _alt;
    HalfEar _azi;
    public:
    Ear(int altPin, int altNeuter, int aziPin, int aziNeuter) : _alt(altPin, altNeuter),
                                                                _azi(aziPin, aziNeuter) {};

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
        finish = finish and this->_azi.step(now);
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
                left(lAltPin, lAltNeuter, lAziPin, lAziNeuter),
                right(rAltPin, rAltNeuter, rAziPin, rAziNeuter) {};

    bool step() {
        bool finish;
        if(move) {
            unsigned long now = millis();
            finish = this->left.step(now);
            finish = finish and this->right.step(now);
            if(finish) {
                this->define_move(move->next);
            }
            return finish;
        } else {
            return true;
        }
    }
    void define_move(struct ears_target * move) {
        this->move = move;
        if (move) {
            this->left.attach();
            this->left.define_move(&move->left);
            this->right.attach();
            this->right.define_move(&move->right);
        } else {
            this->left.detach();
            this->right.detach();
        }
    }
} ears(LeftAltPin, INIT_LEFT_ALT,
       LeftAziPin, INIT_LEFT_AZI,
       RightAltPin, INIT_RIGHT_ALT,
       RightAltPin, INIT_RIGHT_AZI) ;

void setup()
{

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


    server.begin();

    ears.left.attach();
    ears.right.attach();

    ears.left.moveto(0, 0);
    ears.right.moveto(0, 0);
    delay(300);

    ears.left.detach();
    ears.right.detach();


}

//1
void mvt_triste(void)
{
    mvt_table[0] = {
        .left = { .azi = { .dest = 0, .inter = -1 },
                  .alt = { .dest = 0, .inter = -1 },
                },
        .right ={ .azi = { .dest = 0, .inter = -1 },
                  .alt = { .dest = 0, .inter = -1 },
                },
        .next = &mvt_table[1]
    };
    mvt_table[1] = {
        .left = { .azi = { .dest = 0, .inter = 0 },
                  .alt = { .dest = 30, .inter = 5 },
                },
        .right ={ .azi = { .dest = 0, .inter = 0 },
                  .alt = { .dest = 30, .inter = 5 },
                },
        .next = NULL
    };
    ears.define_move(&mvt_table[0]);
}

//2
void mvt_penaud(void)
{
}

//3
void mvt_gauche(void)
{
}

//4
void mvt_droit(void)
{
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
}

void loop()
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  ears.step();

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

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

