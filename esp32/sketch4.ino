// Sketch che invia un topic MQTT utilizzando TLS
/*
  Secure connection example for ESP32 <----> Mosquitto broker (used for MQTT) communcation
  with possible client authentication
  Prerequisite:
  PubSubClient library for Arduino - https://github.com/knolleary/pubsubclient/
  OpenSSL - https://www.openssl.org/
  Mosquitto broker - https://mosquitto.org/
  
  1. step - Generate the certificates
  For generating self-signed certificates please run the following commands:
      
  openssl req -new -x509 -days 365 -extensions v3_ca -keyout ca.key -out ca.crt -subj '/CN=TrustedCA.net'  #If you generate self-signed certificates the CN can be anything
  openssl genrsa -out mosquitto.key 2048
  openssl req -out mosquitto.csr -key mosquitto.key -new -subj '/CN=Mosquitto_borker_adress'    #Its necessary to set the CN to the adress of which the client calls your Mosquitto server (eg. yourserver.com)!!!
  openssl x509 -req -in mosquitto.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out mosquitto.crt -days 365 
  #This is only needed if the mosquitto broker requires a client autentithication (require_certificate is set to true in mosquitto config)
  openssl genrsa -out esp.key 2048
  openssl req -out esp.csr -key esp.key -new -subj '/CN=localhost'
  openssl x509 -req -in esp.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out esp.crt -days 365
  2. Open ca.crt, esp.crt and esp.key with text viewer and copy the values to this WiFiClientSecureClientAuthentication.ino source file into 
  corresponding const char CA_cert[], const char ESP_CA_cert[] and const char ESP_RSA_key[] with escape characters.
  (1-2.) Alternatively you can use the certificates/certificate_generator.sh script 
  for generating and formatting the certificates. Befor you run it, please modify the CN value for your adress, or modify any other settings based on yout requierments.
  3. step - Install and setup your Mosquitto broker
  Follow the instructions from https://mosquitto.org/ and check the manual for the configuration.
  For the Mosquito broker you need ca.crt, mosquitto.key and mosquitto.crt files generated in previous step.
  Recommended to put they in /etc/mosquitto/ca_certificates/ and /etc/mosquitto/certs/
  You need to config Mosquitto broker to use these files (usually /etc/mosquitto/conf.d/default.conf):
  
  listener 8883
  cafile path/to/ca.crt
  keyfile path/to/mosquitto.key
  certfile path/to/mosquitto.crt
  require_certificate true or false #If you need client authentication set it to true
  log_type all  #for logging in /var/log/mosquitto/
  4.Restart the Mosquitto service or start the broker:
  sudo service mosquitto restart
  or
  mosquitto -c /etc/mosquitto/conf.d/default.conf
  
  2021 - Norbert Gal - Apache 2.0 License.
*/

#include <PubSubClient.h>
#include "WiFiClientSecure.h"

const char* CA_cert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "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\n" \
  "-----END CERTIFICATE-----\n";

const char* ESP_CA_cert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "-----END CERTIFICATE-----\n";

const char* ESP_RSA_key= \
  "-----BEGIN PRIVATE KEY-----\n" \
  "-----END PRIVATE KEY-----\n";

const char* ssid        = "Vodafone-A76077447";        // your network SSID (name of wifi network)
const char* password    = "";   // your network password

const char* mqtt_server = "brunomarelli.duckdns.org";  //Adress for your Mosquitto broker server, it must be the same adress that you set in Mosquitto.csr CN field
int port                = 8883;             //Port to your Mosquitto broker server. Dont forget to forward it in your router for remote access
const char* mqtt_user   = "";           //Depends on Mosquitto configuration, if it is not set, you do not need it
const char* mqtt_pass   = "";  //Depends on Mosquitto configuration, if it is not set, you do not need it

WiFiClientSecure client;
PubSubClient mqtt_client(client); 

void setup() {
  Serial.begin(115200);
  delay(100); 

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  //Set up the certificates and keys
  client.setCACert(CA_cert);          //Root CA certificate
  client.setCertificate(ESP_CA_cert); //for client verification if the require_certificate is set to true in the mosquitto broker config
  client.setPrivateKey(ESP_RSA_key);  //for client verification if the require_certificate is set to true in the mosquitto broker config

  mqtt_client.setServer(mqtt_server, port);
}

void loop() {  
  Serial.println("\nStarting connection to server...");
  //if you use password for Mosquitto broker
  //if (mqtt_client.connect("ESP32", mqtt_user , mqtt_pass)) {
  //if you dont use password for Mosquitto broker
  if (mqtt_client.connect("ESP32")) {                       
    Serial.print("Connected, mqtt_client state: ");
    Serial.println(mqtt_client.state());
    //Publsih a demo message to topic LivingRoom/TEMPERATURE with a value of 25
    mqtt_client.publish("LivingRoom/TEMPERATURE", "25");
  }
  else {
    Serial.println("Connected failed!  mqtt_client state:");
    Serial.print(mqtt_client.state());
    Serial.println("WiFiClientSecure client state:");
    char lastError[100];
    client.lastError(lastError,100);  //Get the last error for WiFiClientSecure
    Serial.print(lastError);
  }
  delay(10000);
}





#################################################################################################################################################################################
#Certificate generator for TLS encryption																	#
#################################################################################################################################################################################
openssl req -new -x509 -days 365 -extensions v3_ca -keyout ca.key -out ca.crt -passout pass:1234 -subj '/CN=TrustedCA.net'
#If you generating self-signed certificates the CN can be anything

openssl genrsa -out mosquitto.key 2048
openssl req -out mosquitto.csr -key mosquitto.key -new -subj '/CN=locsol.dynamic-dns.net'
openssl x509 -req -in mosquitto.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out mosquitto.crt -days 365 -passin file:PEMpassphrase.pass
#Mostly the client verifies the adress of the mosquitto server, so its necessary to set the CN to the correct adress (eg. yourserver.com)!!!


#################################################################################################################################################################################
#These certificates are only needed if the mosquitto broker requires a certificate for client autentithication (require_certificate is set to true in mosquitto config).      	#
#################################################################################################################################################################################
openssl genrsa -out esp.key 2048
openssl req -out esp.csr -key esp.key -new -subj '/CN=localhost'
openssl x509 -req -in esp.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out esp.crt -days 365 -passin pass:1234
#If the server (mosquitto) identifies the clients based on CN key, its necessary to set it to the correct value, else it can be blank.

#################################################################################################################################################################################
#For ESP32. Formatting for source code. The output is the esp_certificates.c                                                                                                    #
#################################################################################################################################################################################
echo -en "#include \"esp_certificates.h\"\n\nconst char CA_cert[] = \\\\\n" > esp_certificates.c;
sed -z '0,/-/s//"-/;s/\n/\\n" \\\n"/g;s/\\n" \\\n"$/";\n\n/g' ca.crt >> esp_certificates.c     #replace first occurance of - with "-  ;  all newlnies with \n " \ \n", except last newline where just add ;"
echo "const char ESP_CA_cert[] = \\" >> esp_certificates.c;
sed -z '0,/-/s//"-/;s/\n/\\n" \\\n"/g;s/\\n" \\\n"$/";\n\n/g' esp.crt >> esp_certificates.c     #replace first occurance of - with "-  ;  all newlnies with \n " \ \n", except last newline where just add ;"
echo "const char ESP_RSA_key[] = \\" >> esp_certificates.c;
sed -z '0,/-/s//"-/;s/\n/\\n" \\\n"/g;s/\\n" \\\n"$/";/g' esp.key >> esp_certificates.c     #replace first occurance of - with "-  ;  all newlnies with \n " \ \n", except last newline where just add ;"