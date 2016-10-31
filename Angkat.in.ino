#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Servo.h>

// Update these with values suitable for your network.
byte mac[] = {0x98, 0x4F, 0xEE, 0x00, 0xB0, 0x28};
byte ip[] = {192, 168, 100, 2};
byte dns[] = {8, 8, 8, 8};
byte gateway[] = {192, 168, 100, 1};

int sendMQTT = 0;
bool conn_ok;
char message_buff[200];
char message_buff_call[200];
char *geeknesia_deviceid = "device-09f85271890c5e663c33a7c32f36d3fd";
char *geeknesia_credential = "a164e6d01152d74ead3732d2dde45ade:864c9c7229f711a6be40568ea2982df1";
int lampu = 13;
int buzz = 6;
int itemp = 0;
int sCahaya = A0;
int tCahaya = 0;
int gCahaya = 0;
int tLooping = 0;
int t2Looping = 0;
int mendung = 0;
int cerah = 0;
int servo = 5;

Servo gJemuran;
EthernetClient ethClient;

void callback(char *topic, byte *payload, unsigned int length) {
    int i = 0;
    for (i = 0; i < length; i++) {
        message_buff_call[i] = payload[i];
    }
    message_buff_call[i] = '\0';
    String msgString = String(message_buff_call);
    Serial.println(msgString);

    if (msgString.equals("on")) {
        Serial.println("ON");
        digitalWrite(lampu, HIGH);
    }
    if (msgString.equals("off")) {
        Serial.println("OFF");
        digitalWrite(lampu, LOW);
    }
}

PubSubClient client("geeknesia.com", 1883, callback, ethClient);

void setup() {
    // reset network
    system("ifdown eth0");
    system("ifup eth0");
    // wait for accessing sensor
    delay(2000);
    Serial.begin(9600);
    // Koneksi DHCP
    if (Ethernet.begin(mac) != 1) {
        // Jika gagal, maka koneksi manual
        Ethernet.begin(mac, ip, dns, gateway);
    }
    Serial.print("Running on: ");
    Serial.println(Ethernet.localIP());
    Serial.println("connecting...");
    while (client.connect(geeknesia_deviceid, "iot/live", 0, 0, geeknesia_deviceid) != 1) {
        Serial.println("Error connecting to MQTT");
        delay(2000);
    }
    pinMode(lampu, OUTPUT);
    pinMode(buzz, OUTPUT);
    pinMode(sCahaya, INPUT);
    gJemuran.attach(servo);
}

void senddata(char *topic, char *credential, char *var1, int nilai1) {
    String pubString = "{\"code\":\"";
    pubString += credential;
    pubString += "\",";
    pubString += "\"attributes\":{\"";
    pubString += String(var1) + "\":\"" + nilai1 + "\"";
    pubString += "}}";
    char message_buff[pubString.length() + 1];
    pubString.toCharArray(message_buff, pubString.length() + 1);
    Serial.println(message_buff);
    client.publish("iot/data", message_buff);
}

void loop() {
    if (!client.connected()) {
        client.connect(geeknesia_deviceid, NULL, NULL, "iot/live", 2, 64, geeknesia_deviceid);
    }
    tCahaya = analogRead(sCahaya);
    // Sending data to Geeknesia
    senddata(geeknesia_deviceid, geeknesia_credential, "Cahaya", tCahaya);
    if (sendMQTT == 1) {
        conn_ok = client.connected();
    }
    if (conn_ok == 0) {           //no connection, reconnect
        client.disconnect();
    }
    if (tCahaya < 80) {
        digitalWrite(lampu, HIGH);
        if (tLooping == 0) {
            for (gCahaya = 0; gCahaya <= 95; gCahaya++) {
                gJemuran.write(gCahaya);
                delay(5);
            }
            tone(buzz, 900, 50);
            mendung += 1;
        }
        tLooping = 1;
        t2Looping = 0;
    } else {
        digitalWrite(lampu, LOW);
        if (t2Looping == 0) {
            for (gCahaya = 95; gCahaya >= 0; gCahaya--) {
                gJemuran.write(gCahaya);
                delay(5);
            }
            tone(buzz, 700, 50);
            cerah += 1;
        }
        t2Looping = 1;
        tLooping = 0;
    }
    Serial.println("=============================");
    Serial.print(">> Cerah           = ");
    Serial.print(cerah);
    Serial.println(" kali");
    Serial.print(">> Mendung         = ");
    Serial.print(mendung);
    Serial.println(" kali");
    Serial.print(">> Perubahan Cuaca = ");
    Serial.print(cerah + mendung);
    Serial.print(" kali\n>> Cahaya          = ");
    Serial.print(tCahaya);
    Serial.println(" Cd\n=============================\n");
    delay(1000);
}
