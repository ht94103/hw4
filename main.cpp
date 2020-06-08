#include "mbed.h"

#include "MQTTNetwork.h"

#include "MQTTmbed.h"

#include "MQTTClient.h"

#include "fsl_port.h"

#include "fsl_gpio.h"

#include "mbed_rpc.h"

#define UINT14_MAX        16383

// FXOS8700CQ I2C address

#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0

#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0

#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1

#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses

#define FXOS8700Q_STATUS 0x00

#define FXOS8700Q_OUT_X_MSB 0x01

#define FXOS8700Q_OUT_Y_MSB 0x03

#define FXOS8700Q_OUT_Z_MSB 0x05

#define FXOS8700Q_M_OUT_X_MSB 0x33

#define FXOS8700Q_M_OUT_Y_MSB 0x35

#define FXOS8700Q_M_OUT_Z_MSB 0x37

#define FXOS8700Q_WHOAMI 0x0D

#define FXOS8700Q_XYZ_DATA_CFG 0x0E

#define FXOS8700Q_CTRL_REG1 0x2A

#define FXOS8700Q_M_CTRL_REG1 0x5B

#define FXOS8700Q_M_CTRL_REG2 0x5C

#define FXOS8700Q_WHOAMI_VAL 0xC7

/******* The program can be compiled and may be failed (mbedOS error) after a short period when press restart on K66F.
 * Please press reset again, it should be function well. If not, press again.
 * You should wait until the static state. (Check if connected to wifi and xbee on screen first !)
 * You should see the value (when static state) on screen like 
 * 
 * ...
 * 999.5   // the indicator
 * 0.0117  // x_acc
 * 0.7603  // y_acc
 * 0.6558  // z_acc
 * 999.5   // the indicator
 * 0.0117  // x_acc
 * 0.7480  // y_acc
 * 0.6548  // z_acc
 * ...
 * 
 * then you can run the python program.
 * The python program may fail due to unknown reason. (It may send RPC commands for 5 times in the first run and then receive nothing
 * so the program won't contimue, but it may only send 3 RPC commands for 3 times the next time and then receive nothing.)
 *********/

I2C i2c( PTD9,PTD8);

int m_addr = FXOS8700CQ_SLAVE_ADDR1;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);

void FXOS8700CQ_writeRegs(uint8_t * data, int len);

void log_acc();

float t[3]; 

float ind = 999.5;    // To indicate the sampling process, it changes to 111.5 when samplin (0.1s or 0.5s), the python read the indicator to save the desire data

int wait_ind = 0;    // Use when sampling time = 0.5

int j = 10, p = 0, tilt = 1;

uint8_t who_am_i, data[2], res[6];

int16_t acc16;
// GLOBAL VARIABLES

WiFiInterface *wifi;

InterruptIn btn2(SW2);

InterruptIn btn3(SW3);

RawSerial pc(USBTX, USBRX);

RawSerial xbee(D12, D11);

volatile int message_num = 0;

volatile int arrivedcount = 0;

volatile bool closed = false;

const char* topic = "Mbed";


Thread mqtt_thread(osPriorityNormal);

Thread acc_thread(osPriorityHigh);

Thread xbee_thread(osPriorityLow);

EventQueue mqtt_queue(32 * EVENTS_EVENT_SIZE);

EventQueue acc_queue(32 * EVENTS_EVENT_SIZE);

EventQueue xbee_queue(32 * EVENTS_EVENT_SIZE);

RpcDigitalOut myled1(LED1,"myled1");

int number = 0;

void xbee_rx_interrupt(void);

void xbee_rx(void);

void reply_messange(char *xbee_reply, char *messange);

void check_addr(char *xbee_reply, char *messenger);

void messageArrived(MQTT::MessageData& md) {

      MQTT::Message &message = md.message;

      char msg[300];

      sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);

      printf(msg);

      wait_ms(1000);

      char payload[300];

      sprintf(payload, "Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

      printf(payload);

      ++arrivedcount;

}


void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {

      message_num++;

      MQTT::Message message;

      char buff[100];

      sprintf(buff, "%1.1f\n%1.4f\n%1.4f\n%1.4f\n", ind, t[0], t[1], t[2]);

      message.qos = MQTT::QOS0;

      message.retained = false;

      message.dup = false;

      message.payload = (void*) buff;

      message.payloadlen = strlen(buff) + 1;

      int rc = client->publish(topic, message);

      printf("%s", buff);

}


void close_mqtt() {

      closed = true;

}


int main() {

      pc.baud(9600);

      char xbee_reply[4];

      xbee.baud(9600);

      xbee.printf("+++");

      xbee_reply[0] = xbee.getc();

      xbee_reply[1] = xbee.getc();

      if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){

            pc.printf("enter AT mode.\r\n");

            xbee_reply[0] = '\0';

            xbee_reply[1] = '\0';

      }

      xbee.printf("ATMY 0x178\r\n");

      reply_messange(xbee_reply, "setting MY : 0x178");


      xbee.printf("ATDL 0x187\r\n");

      reply_messange(xbee_reply, "setting DL : 0x187");


      xbee.printf("ATID 0x3\r\n");

      reply_messange(xbee_reply, "setting PAN ID : 0x3");


      xbee.printf("ATWR\r\n");

      reply_messange(xbee_reply, "write config");


      xbee.printf("ATMY\r\n");

      check_addr(xbee_reply, "MY");


      xbee.printf("ATDL\r\n");

      check_addr(xbee_reply, "DL");


      xbee.printf("ATCN\r\n");

      reply_messange(xbee_reply, "exit AT mode");

      xbee.getc();

      xbee_thread.start(callback(&xbee_queue, &EventQueue::dispatch_forever));

      wifi = WiFiInterface::get_default_instance();

      if (!wifi) {

            printf("ERROR: No WiFiInterface found.\r\n");

            return -1;

      }



      printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);

      int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);

      if (ret != 0) {

            printf("\nConnection error: %d\r\n", ret);

            return -1;

      }

      xbee.attach(xbee_rx_interrupt, Serial::RxIrq);


      NetworkInterface* net = wifi;

      MQTTNetwork mqttNetwork(net);

      MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);


      const char* host = "192.168.0.111";

      printf("Connecting to TCP network...\r\n");

      int rc = mqttNetwork.connect(host, 1883);

      if (rc != 0) {

            printf("Connection error.");

            return -1;

      }

      printf("Successfully connected!\r\n");


      MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

      data.MQTTVersion = 3;

      data.clientID.cstring = "Mbed";


      if ((rc = client.connect(data)) != 0){

            printf("Fail to connect MQTT\r\n");

      }

      if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){

            printf("Fail to subscribe\r\n");

      }


      mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));

      acc_thread.start(callback(&acc_queue, &EventQueue::dispatch_forever)); 

      Ticker log_accTicker;

      log_accTicker.attach(acc_queue.event(&log_acc), 0.1f);   //To calculate acc every 0.1s

      Ticker mqttTicker;

      mqttTicker.attach(mqtt_queue.event(&publish_message, &client), 0.1f);   //Send data to client every 0.1s, and client read the indicator to determine the sampling time

      btn3.rise(&close_mqtt);

      pc.printf("start\r\n");

     int num = 0;

      while (num != 5) {

            client.yield(100);

            ++num;

      }


      while (1) {

            if (closed) break;

            wait(0.5);

      }


      printf("Ready to close MQTT Network......\n");


      if ((rc = client.unsubscribe(topic)) != 0) {

            printf("Failed: rc from unsubscribe was %d\n", rc);

      }

      if ((rc = client.disconnect()) != 0) {

      printf("Failed: rc from disconnect was %d\n", rc);

      }


      mqttNetwork.disconnect();

      printf("Successfully closed!\n");

      return 0;

}

void log_acc() {
   uint8_t who_am_i, data[2], res[6];
   int16_t acc16;
   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);
   // Get the slave address

   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);
   
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[0] = ((float)acc16) / 4096.0f;

      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[1] = ((float)acc16) / 4096.0f;

      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[2] = ((float)acc16) / 4096.0f;

/***** To determine the moment if tilt changes from < 45 degree to >= 45 degree *****/ 
      if (t[2]*t[2] <= (t[0]*t[0] + t[1]*t[1] + t[2]*t[2])/2){
            p = tilt;
            tilt = 1;
                  if ((j >= 9) && (p != 1)){
                        j = 0;
                  }
      }
      else {
            p = tilt;
            tilt = 0;
      }

/***** To change the indicator and increase number use in xbee every time *****/

      if (j <= 9){

            if (ind < 500){
                  ind = 999.5;
            }
            else {
                  ind = 111.5;
            }
            number++;
            j++;
            wait_ind = 0;
            if (j > 9){
                  return;
            }
      }
         
/***** To increase or reset the indicator and increase number use in xbee  *****/  

      else {
            if (wait_ind == 4){
                  if (ind < 500){
                        ind = 999.5;
                  }
                  else {
                        ind = 111.5;
                  }                 
            }  
            wait_ind++;
            number++;        
      }
      if (wait_ind >= 5){
                  wait_ind = 0;
      }
      
}


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}


void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}



void xbee_rx_interrupt(void)

{

  xbee.attach(NULL, Serial::RxIrq); // detach interrupt

  xbee_queue.call(&xbee_rx);

}


void xbee_rx(void)

{

  char buf[100] = {0};

  char outbuf[100] = {0};

  while(xbee.readable()){

    for (int i=0; ; i++) {

      char recv = xbee.getc();

      if (recv == '\r') {

        break;

      }

      buf[i] = recv; 

    }

    RPC::call(buf, outbuf);

    xbee.printf("%d\r\n", number);

    number = 0;  // reset number every time when RPC command arrived


  }

  xbee.attach(xbee_rx_interrupt, Serial::RxIrq); // reattach interrupt

}


void reply_messange(char *xbee_reply, char *messange){

  xbee_reply[0] = xbee.getc();

  xbee_reply[1] = xbee.getc();

  xbee_reply[2] = xbee.getc();

  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){

    pc.printf("%s\r\n", messange);

    xbee_reply[0] = '\0';

    xbee_reply[1] = '\0';

    xbee_reply[2] = '\0';

  }

}


void check_addr(char *xbee_reply, char *messenger){

  xbee_reply[0] = xbee.getc();

  xbee_reply[1] = xbee.getc();

  xbee_reply[2] = xbee.getc();

  xbee_reply[3] = xbee.getc();

  pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);

  xbee_reply[0] = '\0';

  xbee_reply[1] = '\0';

  xbee_reply[2] = '\0';

  xbee_reply[3] = '\0';

}
