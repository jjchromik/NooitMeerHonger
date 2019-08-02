#include <Servo.h>

Servo myservo;  // create servo object to control a servo

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid     = "";
const char* password = "";


unsigned long feedIntervalFullMsec = 28800000; // 8 hours
unsigned long feedIntervalMiniMsec = 3600000; // 1 hour
int foodGiven = 0;
unsigned long nextFullFeedTime = 0;
unsigned long nextMiniFeedTime = 0;

int feedRate = 75; // 90-15
int feedReversal = 110; // 90+20
int neutralRate = 90;

int feedTimeFullCount = 30; // total count per day 
int feedIntervalPortion = feedTimeFullCount/3;

//// For static IP address:
//WiFi.begin(ssid, password);
//IPAddress ip(192,168,1,200);   
//IPAddress gateway(192,168,1,254);   
//IPAddress subnet(255,255,255,0);   
//WiFi.config(ip, gateway, subnet);

// Set web server port number to 80
ESP8266WebServer  server(80);

void testForward(){
  String message = "";
  int newTime = 1;
  int newFeedRate = 70;
  message += "Testing...\n"; 
  
  if (server.arg("feedRate") != ""){ 
    newFeedRate = server.arg("feedRate").toInt();
    message += "feedRate = ";
    message += server.arg("feedRate");
  }
  if (server.arg("time") != ""){ 
    newTime =  server.arg("time").toInt();
    message += "\ntime = ";
    message += server.arg("time");
  }
  
  myservo.write(newFeedRate);
  delay(newTime*1000);
  myservo.write(neutralRate);

  server.send(200, "text/plain", message);          //Returns the HTTP response  

}



void singleFeed(int times){
  for (int cnt = 0; cnt < times; cnt++)
      {
        myservo.write(feedRate);
        delay(1000);
        myservo.write(feedReversal);
        delay(500);
        myservo.write(feedRate);
        delay(500);
      }
      myservo.write(neutralRate);
}

void helpFeeder(){
  String message = "";
  message += "This help function gives the current parameters and provides the names to other functions.\n\n";
  message += "#####################################\n";
  message += "######## Current parameters #########\n";
  message += "#####################################\n";
  message += "feedRate* = ";
  message += String(feedRate);
  message += "\nfeedReversal* = ";
  message += String(feedReversal);
  message += "\nfeedTimeFullCount* = ";
  message += String(feedTimeFullCount);
  message += "\n\n#####################################\n";
  message += "############# Functions #############\n";
  message += "#####################################\n";  
  message += "feed?Time=2 - feeds the cat for 2 units of time. A unit of time is defined by 1s feeding, followed by 0,5s in reverse and 0,5s forward again.\n";
  message += "reset - resets the feeder to the default settings.\n";
  message += "parameters?parameterName=Value&anotherParameterName=Value2 - change the parameters of the feeder. The editable parameters are marked above with a *";
  server.send(200, "text/plain", message);          //Returns the HTTP response  
}

void resetFeeder(){
  nextFullFeedTime = millis()+feedIntervalFullMsec;
  nextMiniFeedTime = 0;
  feedRate = 75; // 90-15
  feedReversal = 110; // 90+20
  neutralRate = 90;
  foodGiven = 0;
  feedTimeFullCount = 30;
  feedIntervalPortion = feedTimeFullCount/3;
  server.send(200, "text/plain", "The feeder was reset ");          //Returns the HTTP response
}

void setParameters(){
  String message = ""; 
  if (server.arg("feedRate") != ""){     //Parameter found
    message += "feedRate set to ";
    message += server.arg("feedRate");
    message += "\n";
    feedRate = server.arg("feedRate").toInt();
  } 
  if (server.arg("feedReversal") != ""){     //Parameter found
    message += "feedReversal set to ";
    message += server.arg("feedReversal");
    message += "\n";
    feedReversal = server.arg("feedReversal").toInt();
  }
  if (server.arg("feedTimeFullCount") != ""){     //Parameter found
    message += "feedTimeFullCount set to ";
    message += server.arg("feedTimeFullCount");
    message += "\n";
    feedTimeFullCount = server.arg("feedTimeFullCount").toInt();
  }  
  
  if (message == "") {
    message += "No parameters changed.";
  }
    server.send(200, "text/plain", message);          //Returns the HTTP response
}

void feedCat() { //Handler
  String message = "";
  int feedAmount;
  if (server.arg("Time")== ""){     //Parameter not found
    message = "Time argument not found";
  }else{     //Parameter found
    message = "Time Argument = ";
    message += server.arg("Time");     //Gets the value of the query parameter
    feedAmount =  server.arg("Time").toInt();
  }

  unsigned long timeNow = millis();
  
  if (foodGiven+feedAmount <= feedIntervalPortion && (long)(timeNow - nextMiniFeedTime) >= 0)
  {
    foodGiven += feedAmount;
  // Move the servo for that amount of time
  singleFeed(feedAmount);
//  for (int cnt = 0; cnt < feedAmount; cnt++)
//    {
//      myservo.write(feedRate);
//      delay(1000);
//      myservo.write(feedReversal);
//      delay(500);
//      myservo.write(feedRate);
//      delay(500);
//    }
//  myservo.write(neutralRate);
  nextMiniFeedTime = timeNow + feedIntervalMiniMsec;
  } else {
  message += "\nThe cat was recently fed or the quantity you request is too high."  ;
  }

  server.send(200, "text/plain", message);          //Returns the HTTP response

}

void handleArgs() { //Handler

String message = "Number of args received:";
message += server.args();            //Get number of parameters
message += "\n";                            //Add a new line

for (int i = 0; i < server.args(); i++) {

message += "Arg n" + (String)i + " = ";   //Include the current iteration value
message += server.argName(i) + ": ";     //Get the name of the parameter
message += server.arg(i) + "\n";              //Get the value of the parameter

} 

server.send(200, "text/plain", message);       //Response to the HTTP request

}

// Variable to store the HTTP request
String header;

// Assign output variables to GPIO pins
const int pinservo = 2;

unsigned long timeNow;

void setup() {
  Serial.begin(115200);
  myservo.attach(2);  // attaches the servo on GIO2 to the servo object
  delay(10);
  myservo.write(90);  // 90 is the middle

  // Set the next full feeding time in 8 hours so it doesn't dump all the food every time after a reset. 
  timeNow = millis();
  nextFullFeedTime = timeNow + feedIntervalFullMsec;
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.printf("Chip ID = %08X\n", ESP.getChipId());

  // Handle the following functions
  server.on("/genericArgs", handleArgs); //Associate the handler function to the path
  server.on("/feed", feedCat); //Associate the handler function to the path
  server.on("/reset", resetFeeder); //Associate the handler function to the path
  server.on("/parameters", setParameters);
  server.on("/", helpFeeder);
  server.on("/help", helpFeeder);

  server.on("/test", testForward);

}

void loop(){
  server.handleClient();  
  int feedAmount = 0;
  timeNow = millis();
  if (timeNow > nextFullFeedTime) {
    if (feedIntervalPortion-foodGiven > 0) {
      feedAmount = feedIntervalPortion-foodGiven;
      singleFeed(feedAmount);
//      for (int cnt = 0; cnt < feedAmount; cnt++)
//      {
//        myservo.write(feedRate);
//        delay(1000);
//        myservo.write(feedReversal);
//        delay(500);
//        myservo.write(feedRate);
//        delay(500);
//      }
//      myservo.write(neutralRate);
    }
    foodGiven = 0;
    nextFullFeedTime = timeNow + feedIntervalFullMsec;
  }
}
