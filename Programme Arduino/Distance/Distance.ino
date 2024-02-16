
const int trigPin = 2; // Trigger (émission)
const int echoPin = 3; // Echo (réception)

long duree; // Durée de l'echo

void setup() {
    pinMode(trigPin, OUTPUT); // Configuration du port du Trigger comme une SORTIE
    pinMode(echoPin, INPUT); // Configuration du port de l'Echo comme une ENTREE
    Serial.begin(9600) ; // Démarrage de la communication série à 9600 bits/s
}

void loop() {
    uint8_t distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duree = pulseIn(echoPin, HIGH);
    distance = duree*0.034/2;

    if(distance <200 && distance >4)
    {
      Serial.println(distance) ;
    }

    delay(250); // Délai d'attente pour éviter d'afficher trop de résultats à la seconde
}
