//arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .

#include <ESP8266WiFi.h>
#include <Servo.h>

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
    unsigned long _last;
    int _pin;

    void _moveto(int angle, unsigned long now) {
        this->_cur = angle;
        this->_last = now;
        if (!this->_s.attached()) {
            /* ERROR */
        }
        this->_s.write(angle);
    }

    public:
    HalfEar(int pin) : _pin(pin) {};

    void attach() {
        this->_s.attach(this->_pin);
    }
    void detach() {
        this->_s.detach();
    }

    /*void moveto(int angle, unsigned long now) {
        this->_moveto(angle, now);
    }*/
    void moveto(int angle) {
        this->_moveto(angle, millis());
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
    Ear(int altPin, int aziPin) : _alt(altPin), _azi(aziPin) {};

    void attach() {
        this->_azi.attach();
        this->_alt.attach();
    }
    void detach() {
        this->_azi.detach();
        this->_alt.detach();
    }


    void moveto(int azimuth, int altitude) {
        this->_azi.moveto(azimuth);
        this->_alt.moveto(altitude);
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

    ears(int lAlt, int lAzi, int rAlt, int rAzi) : left(lAlt, lAzi), right(rAlt, rAzi) {};

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
} ears(LeftAltPin, LeftAziPin, RightAltPin, RightAltPin) ;

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

    ears.left.moveto(INIT_LEFT_AZI, INIT_LEFT_ALT);
    ears.right.moveto(INIT_RIGHT_AZI, INIT_RIGHT_ALT);
    delay(300);

    ears.left.detach();
    ears.right.detach();


}

//1
void mvt_triste(void)
{
    mvt_table[0] = {
        .left = { .azi = { .dest = INIT_LEFT_AZI, .inter = -1 },
                  .alt = { .dest = INIT_LEFT_ALT, .inter = -1 },
                },
        .right ={ .azi = { .dest = INIT_RIGHT_AZI, .inter = -1 },
                  .alt = { .dest = INIT_RIGHT_ALT, .inter = -1 },
                },
        .next = &mvt_table[1]
    };
    mvt_table[1] = (struct ears_target) {
        .left = { .azi = { .dest = INIT_LEFT_AZI, .inter = 0 },
                  .alt = { .dest = INIT_LEFT_ALT+30, .inter = 5 },
                },
        .right ={ .azi = { .dest = INIT_RIGHT_AZI, .inter = 0 },
                  .alt = { .dest = INIT_RIGHT_ALT+30, .inter = 5 },
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

  ears.step();

  // Return the response
 String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
        header += "<head>";
        header += "<meta charset=\"utf-8\">";
        header += "<title>Neko_Mimi</title>";

header += "<style>";
header += "html,";
header += "body {";
header += "  background: -webkit-linear-gradient(left, #e0e3ea 50%, #e3e6ec 50%);";
header += "  background: -moz-linear-gradient(left, #e0e3ea 50%, #e3e6ec 50%);";
header += "  background: linear-gradient(left, #e0e3ea 50%, #e3e6ec 50%);";
header += "  background-size: 10px 10px;";
header += "}";
header += "#main {";
header += "  margin-bottom: 2em;";
header += "  vertical-align:middle;";
header += "}";
header += ".page-header {";
header += "  border-bottom-color: #b0b4bc;";
header += "  font-size: 200%;";
header += "  text-align:center;";
header += "}";
header += ".ios-dl .legend {";
header += "  display: block;";
header += "  padding: 0;";
header += "  margin: 20px 0px 0px;";
header += "  font-size: 21px;";
header += "  line-height: 40px;";
header += "  color: #8B91A5;";
header += "  font-weight: bold;";
header += "}";
header += ".ios-dl .definition-group {";
header += "  background: #fff;";
header += "  border-radius: 10px;";
header += "  border: 1px solid #b0b4bc;";
header += "  box-shadow: 0 1px 1px 0 white, 0 1px 1px 0 #d9dbdf inset;";
header += "}";
header += ".ios-dl dl {";
header += "  padding: 1em;";
header += "  margin: 0px;";
header += "  background: transparent;";
header += "  border-top: 1px solid #d9dbdf;";
header += "}";
header += ".ios-dl dl:first-child {";
header += "  border: none;";
header += "}";
header += ".ios-dl dt {";
header += "  text-align: center;";
header += "  font-size: 250%;";
header += "  font-weight: bold;";
header += "}";
header += "a:hover, a:visited, a:link, a:active";
header += "{";
header += "  text-decoration: none;";
header += "  display:block;width:100%;height:100%;";
header += "  color:  #A0A0A0;";
header += "  font-size: 150%;";
header += "}";
header += "img { ";
header += "  vertical-align:middle;";
header += "}";
header += "</style>";
header += "</head>";

  client.print(header);

String body = "  <body>";
body += "    <div id=\"main\" class=\"container-fluid\">";
body += "  <div class=\"page-header\">";
body += "  </div>";
body += "  <div class=\"row-fluid\">";
body += "    <div class=\"span12\">";
body += "      <div class=\"main ios-dl\">";
body += "        <span class=\"legend\"></span>";
body += "        <div class=\"definition-group\">";

  client.print(body);

String img1 = "          <dl class=\"dl-horizontal\">";
img1 += "            <dt><a href=\"/mvt1/\">Triste</a></dt>";
img1 += "          </dl>";

  client.print(img1);

String img2 = "          <dl class=\"dl-horizontal\">";
img2 += "            <dt><a href=\"/mvt2/\">Penaud</a></dt>";
img2 += "          </dl>";

  client.print(img2);

  String img3 = "          <dl class=\"dl-horizontal\">";
img3 += "            <dt><a href=\"/mvt3/\">Oreille Gauche</a></dt>";
img3 += "          </dl>";

  client.print(img3);

String img4 = "          <dl class=\"dl-horizontal\">";
img4 += "            <dt><a href=\"/mvt4/\">Oreille Droite</a></dt>";
img4 += "          </dl>";

  client.print(img4);

  String img5 = "          <dl class=\"dl-horizontal\">";
img5 += "            <dt><a href=\"/mvt5/\">Aux aguets</a></dt>";
img5 += "          </dl>";

  client.print(img5);

String img6 = "          <dl class=\"dl-horizontal\">";
img6 += "            <dt><a href=\"/mvt6/\">Content</a></dt>";
img6 += "          </dl>";

  client.print(img6);

  String img7 = "          <dl class=\"dl-horizontal\">";
img7 += "            <dt><a href=\"/mvt7/\">écoute</a></dt>";
img7 += "          </dl>";

  client.print(img7);

    String img8 = "          <dl class=\"dl-horizontal\">";
img8 += "            <dt><a href=\"/mvt8/\">Surprise</a></dt>";
img8 += "          </dl>";

  client.print(img8);

    String img9 = "          <dl class=\"dl-horizontal\">";
img9 += "            <dt><a href=\"/mvt9/\">Baissées</a></dt>";
img9 += "          </dl>";

  client.print(img9);

    String img10 = "          <dl class=\"dl-horizontal\">";
img10 += "            <dt><a href=\"/mvt10/\">Tournées</a></dt>";
img10 += "          </dl>";

  client.print(img10);

      String img11 = "          <dl class=\"dl-horizontal\">";
img11 += "            <dt><a href=\"/mvt11/\">Reset Origine</a></dt>";
img11 += "          </dl>";

  client.print(img11);

String footer = "        </div>";
footer += "      </div>";
footer += "    </div>";
footer += "  </div>";
footer += "</body>";
footer += "</html>\n";

  client.print(footer);

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

