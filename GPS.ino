#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPS module connections (use appropriate pins)
#define RX_PIN 4
#define TX_PIN 3
#define BAUD_RATE 9600

// Create software serial for GPS
SoftwareSerial gpsSerial(RX_PIN, TX_PIN);

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Define a structure to hold GPS data
struct GPSData {
    String time;
    double latitude;
    double longitude;
    double velocity;

    void update(const TinyGPSPlus &gps) {
        if (gps.time.isUpdated()) {
            time = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
        }
        if (gps.location.isUpdated()) {
            latitude = gps.location.lat();
            longitude = gps.location.lng();
        }
        if (gps.speed.isUpdated()) {
            velocity = gps.speed.kmph();
        }
    }

    void print() {
        Serial.print("Time: "); Serial.println(time);
        Serial.print("Latitude: "); Serial.println(latitude, 6);
        Serial.print("Longitude: "); Serial.println(longitude, 6);
        Serial.print("Velocity: "); Serial.print(velocity); Serial.println(" km/h");
    }
};

GPSData gpsData;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    while (!Serial);

    // Initialize GPS serial communication
    gpsSerial.begin(BAUD_RATE);

    Serial.println("GPS Module Test");
}

void loop() {
    // Process data from GPS module
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        gps.encode(c);
    }

    // Update GPS data object
    gpsData.update(gps);

    // Print data for debugging (remove in production)
    gpsData.print();

    // Add a small delay
    delay(1000);
}
