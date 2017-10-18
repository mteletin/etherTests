#include <SPI.h>
#include <Ethernet.h>
#include <Wstring.h>

// Enter a MAC address and IP address for your controller below.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//// Our IP address
IPAddress ip(172,17,16,240);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

char * action_text = "";

int   pins[8] =
{
  14,
  15,
  16,
  17,

  18,
  19,
  20,
  21
};

char* paths[8][2] =
{
  {"sw1/on", "sw1/off"},
  {"sw2/on", "sw2/off"},
  {"sw3/on", "sw3/off"},
  {"sw4/on", "sw4/off"},

  {"sw5/on", "sw5/off"},
  {"sw6/on", "sw6/off"},
  {"sw7/on", "sw7/off"},
  {"sw8/on", "sw8/off"}
};

void printIPAddress()
{
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    if (thisByte <3) Serial.print(".");
  }
}

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) { } // wait for serial port to connect.
  Serial.println(F("Serial on."));

  for(int i = 0; i<8; i++)
  {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }
  Serial.println(F("Using pins 14 - 21 as digital output. Using active LOW on all 8 relays."));
  Serial.println(F("DHCP setup..."));
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.println("Using failover fixed IP");
    Ethernet.begin(mac, ip);
  }

  // print your local IP address:
  Serial.print(F("The device IP address is:")); printIPAddress(); Serial.println();
  
  server.begin();
  Serial.println(F("Web Server listening on port 80."));
  Serial.println();
  Serial.println(F("Available URL commands are:"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw1/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw1/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw2/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw2/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw3/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw3/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw4/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw4/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw5/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw5/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw6/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw6/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw7/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw7/off "));

  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw8/on"));
  Serial.print(F("http://")); printIPAddress(); Serial.println(F("/?sw8/off "));
  // end of setup
}

// how much serial data we expect before a newline
const unsigned int MAX_INPUT = 100;

// the maximum length of paramters we accept
const int MAX_PARAM = 20;

void switchOn(int index)
{
  digitalWrite(pins[index], LOW);
  action_text = paths[index][0];
}

void switchOff(int index)
{
  digitalWrite(pins[index], HIGH);
  action_text = paths[index][1];
}

// Example GET line: GET /?foo=bar HTTP/1.1
void processGet (const char * data)
{
  String dat = String(data);
  Serial.println("1");
  // find where the parameters start
  const char * paramsPos = strchr (data, '?');
  if (paramsPos == NULL)
    return;  // no parameters
  Serial.println("2");
  // find the trailing space
  const char * spacePos = strchr (paramsPos, ' ');
  if (spacePos == NULL)
    return;  // no space found
  Serial.println("3");  
  // work out how long the parameters are
  int paramLength = spacePos - paramsPos - 1;
  Serial.println("4");
  // see if too long
  if (paramLength >= MAX_PARAM)
    return;  // too long for us
  Serial.println("5");  
  // copy parameters into a buffer
  char param [MAX_PARAM];
  memcpy (param, paramsPos + 1, paramLength);  // skip the "?"
  param [paramLength] = 0;  // null terminator
  Serial.println("6");
  Serial.println(param);
  
  for(int i=0; i<8; i++)
  {
    if (strcmp( param, paths[i][0] ) == 0) switchOn(i);
    else if (strcmp( param, paths[i][1] ) == 0) switchOff(i);
  }
  Serial.println("7");
//  // do things depending on argument (GET parameters)
//  if (strcmp (param, "foo") == 0)
//    Serial.println (F("Activating foo"));
//  else if (strcmp (param, "bar") == 0)
//    Serial.println (F("Activating bar"));

}  // end of processGet

// here to process incoming serial data after a terminator received
void processData (const char * data)
{
  Serial.println (data);
  if (strlen (data) < 4)
    return;

  if (memcmp (data, "GET ", 4) == 0)
    processGet (&data [4]);
}  // end of processData

bool processIncomingByte (const byte inByte)
{
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;
  switch (inByte)
  {
    case '\n':   // end of text
      input_line [input_pos] = 0;  // terminating null byte
      if (input_pos == 0)
        return true;   // got blank line
      // terminator reached! process input_line here ...
      processData (input_line);
      
      // reset buffer for next time
      input_pos = 0;  
      break;

    case '\r':   // discard carriage return
      break;

    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;
  }  // end of switch
  return false;    // don't have a blank line yet
} // end of processIncomingByte  


void loop() 
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) 
  {
    Serial.println(F("Client connected"));
    // an http request ends with a blank line
    boolean done = false;
    while (client.connected() && !done) 
    {
      while (client.available () > 0 && !done)
        done = processIncomingByte (client.read ());
    }  // end of while client connected

    // send a standard http response header
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: text/html"));
    client.println(F("Connection: close"));  // close after completion of the response
    client.println();   // end of HTTP header
    client.println(F("<!DOCTYPE HTML>"));
    client.println(F("<html>"));
    client.println(F("<head>"));
    client.println(F("<title>Test Machine</title>"));
    client.println(F("</head>"));
    client.println(F("<body>"));
    client.println(F("<h1>Relay tests HTTP Gateway</h1>"));
    client.println(F("<p>Action performed:<br>"));
    client.println(action_text);
    client.println(F("</body>"));
    client.println(F("</html>"));

    // give the web browser time to receive the data
    delay(10);
    
    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }  // end of got a new client
}  // end of loop
