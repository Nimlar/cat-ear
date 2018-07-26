#include <ESP8266WiFi.h>
#include <Servo.h>

//////////////////////
// Pins Definitions //
//////////////////////

const char WiFiAPPSK[] = "oreilles"; //password wifi

#define       LeftVerPin                     0  //D3 blanc
#define       LeftAngPin                     2  //D4 bleu
#define       RightVerPin                    16 //D0 Rouge
#define       RightAngPin                    5  //D1 noir


#define       INIT_EARS_POS                  90
#define       INIT_LEFT_VER_POS              50 
#define       INIT_RIGHT_VER_POS             85 
#define       INIT_LEFT_ANG_POS              100
#define       INIT_RIGHT_ANG_POS             100

Servo LeftVer;
Servo LeftAng;
Servo RightVer;
Servo RightAng;

/////////////////////
// Wifi Definitions //
/////////////////////

WiFiServer server(80);

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
 
  LeftVer.attach(LeftVerPin);
  RightVer.attach(RightVerPin);
  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);

  LeftVer.write(INIT_LEFT_VER_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  LeftAng.write(INIT_LEFT_ANG_POS);
  RightAng.write(INIT_RIGHT_ANG_POS);
  delay(300);

  LeftAng.detach();
  RightAng.detach();
  LeftVer.detach();
  RightVer.detach();  

  
}

//1
void mvt_triste(void)
{
  const byte MaxAngleShift = 30;
  unsigned int MoveDelay = 5;
  unsigned int PosDelay = 10000;

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

  LeftVer.write(INIT_LEFT_VER_POS);
  RightVer.write(INIT_RIGHT_VER_POS);
  delay(500);

  LeftVer.detach();
  RightVer.detach();
}

//2
void mvt_penaud(void)
{
   const byte MaxAngleShift = 100;
  const byte MaxAngleShift2 = 20;
  unsigned int MoveDelay = 10;
  unsigned int PosDelay = 5000;

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
void mvt_gauche(void)
{
	
  unsigned int MoveDelay = 10;
  byte AnglePos;
  
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
void mvt_droit(void)
{
  unsigned int MoveDelay = 10;
  byte AnglePos;
  
  RightAng.attach(RightAngPin);
  for(byte i=0; i <= 50; i++)
  {
    RightAng.write(INIT_RIGHT_ANG_POS - i);
    delay(3);
  }
  RightAng.detach();

  AnglePos = -INIT_RIGHT_VER_POS;
  RightVer.attach(RightVerPin);
  for(byte i=0; i <= 50; i++)
  {
    RightVer.write(AnglePos);
    AnglePos = INIT_RIGHT_VER_POS - i;
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
void mvt_aguet(void)
{
  unsigned int MoveDelay = 10;
  byte AngleLeftPos;
  byte AngleRightPos;

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
void mvt_content(void)
{
  const byte MaxAngleShift = 50;
  unsigned int PosDelay = 300;

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
void mvt_ecoute(void)
{

  const byte MaxAngleShift = 100;
  unsigned int MoveDelay = 10;
  unsigned int PosDelay = 500;



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
void mvt_surprise(void)
{

  const byte MaxAngleShift = 35;
  
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
void mvt_baisse(void)
{
  const byte MaxAngleShift = 30;
  unsigned int MoveDelay = 5;
  unsigned int PosDelay = 500;

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
void mvt_tourne(void)
{
  const byte MaxAngleShift = 100;
  unsigned int MoveDelay = 10;
  unsigned int PosDelay = 500;

  
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
void mvt_reset(void)
{
  LeftAng.attach(LeftAngPin);
  RightAng.attach(RightAngPin);
  LeftVer.attach(LeftVerPin);
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

