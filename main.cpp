#include "mbed.h"

#include "MQTTNetwork.h"

#include "MQTTmbed.h"

#include "MQTTClient.h"

#include "mbed.h"

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

I2C i2c( PTD9,PTD8);


int m_addr = FXOS8700CQ_SLAVE_ADDR1;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);

void FXOS8700CQ_writeRegs(uint8_t * data, int len);

void log_acc();

float t[3]; 

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


Thread mqtt_thread;

Thread acc_thread;

Thread xbee_thread;

EventQueue mqtt_queue(32 * EVENTS_EVENT_SIZE);

EventQueue acc_queue(32 * EVENTS_EVENT_SIZE);

EventQueue xbee_queue(32 * EVENTS_EVENT_SIZE);

RpcDigitalOut myled1(LED1,"myled1");

int num = 0

//Timer timer_acc;

//Timer timer_wait;

void xbee_rx_interrupt(void);

void xbee_rx(void);

void reply_messange(char *xbee_reply, char *messange);

void check_addr(char *xbee_reply, char *messenger);



void messageArrived(MQTT::MessageData& md) {
      
      //Timer timer_wait;

      //timer_wait.start();

      MQTT::Message &message = md.message;

      char msg[300];

      sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);

      printf(msg);

      wait_ms(1000);
      /*while(1){
            if (timer_wait.read() > 1){
                  timer_wait.reset();
                  break;
            }
      }*/

      char payload[300];

      sprintf(payload, "Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

      printf(payload);

      ++arrivedcount;

}


void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {

      Timer timer_num;

      Timer timer_wait;

      timer_num.start();

      timer_wait.start();

      int j = 10, p = 0, tilt = 1;

      while(1){

      if (timer_num.read() > 1.0){
            timer_num.reset();
            num = 0;
      }

      message_num++;

      MQTT::Message message;

      char buff[100];

      log_acc();

      sprintf(buff, "FXOS8700Q ACC: X=%1.4f(%x%x) Y=%1.4f(%x%x) Z=%1.4f(%x%x)\r\n",\

            t[0], res[0], res[1],\

            t[1], res[2], res[3],\

            t[2], res[4], res[5]\

      );


      message.qos = MQTT::QOS0;

      message.retained = false;

      message.dup = false;

      message.payload = (void*) buff;

      message.payloadlen = strlen(buff) + 1;

      int rc = client->publish(topic, message);


      printf("rc:  %d\r\n", rc);

      printf("Puslish message: %s\r\n", buff);

      if (AccData[i][2]*AccData[i][2] < (AccData[i][0]*AccData[i][0] + AccData[i][1]*AccData[i][1] + AccData[i][2]*AccData[i][2])/2){
            p = tilt;
            tilt = 1;
                  //j = 0;
                  if ((j == 10) && (p != 1)){
                        j = 0;
                  }
            }
      else {
            p = tilt;
            tilt = 0;
      }

      if ((j < 10) || (tilt == 1)){
                  while(1){
                        if (timer_wait.read() > 0.1){
                              timer_wait.reset();
                              break;
                        }
                  }
                  num++;
                  j++;
            }
            if (j == 10 && tilt == 1){
      `           while(1){
                  if (timer_wait.read() > 0.5){
                        timer_wait.reset();
                        break;
                  }
                  num++;
            }            
      }

      else {
            while(1){
                  if (timer_wait.read() > 0.5){
                        timer_wait.reset();
                        break;
                  }
                  num++;
            }


      /*if (AccData[i][2]*AccData[i][2] < (AccData[i][0]*AccData[i][0] + AccData[i][1]*AccData[i][1] + AccData[i][2]*AccData[i][2])/2){
            tilt = 1;
            while(1){
            if (timer_wait.read() > 0.1){
                  timer_wait.reset();
                  break;
            }
            num++;
            i++;
      }
      else {
            while(1){
            if (timer_wait.read() > 0.5){
                  timer_wait.reset();
                  break;
            }
            num++;
      }*/

      //wait(0.5);
      /*while(1){
            if (timer_wait.read() > 0.5){
                  timer_wait.reset();
                  break;
            }*/
      

}


void close_mqtt() {

      closed = true;

}


int main() {

      pc.baud(9600);

      xbee.baud(9600);

      char xbee_reply[4];

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



      NetworkInterface* net = wifi;

      MQTTNetwork mqttNetwork(net);

      MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);


      //TODO: revise host to your ip

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

      //Ticker log_accTicker;

      //log_accTicker.attach(acc_queue.event(&log_acc), 0.1f);

      //btn2.rise(mqtt_queue.event(&publish_message, &client));

      mqtt_queue.event(&publish_message, &client);
     


      btn3.rise(&close_mqtt);

      pc.printf("start\r\n");

      xbee_thread.start(callback(&xbee_queue, &EventQueue::dispatch_forever));


  // Setup a serial interrupt function of receiving data from xbee

      xbee.attach(xbee_rx_interrupt, Serial::RxIrq);


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
   LED = !LED;

   uint8_t who_am_i, data[2], res[6];
   int16_t acc16;

   // Enable the FXOS8700Q
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
      /*pc.print("FXOS8700Q ACC: X=%1.4f(%x%x) Y=%1.4f(%x%x) Z=%1.4f(%x%x)\r\n",\

            t[0], res[0], res[1],\

            t[1], res[2], res[3],\

            t[2], res[4], res[5]\

      );*/
      
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

  queue.call(&xbee_rx);

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

      buf[i] = recv; //pc.putc(recv);

    }

    RPC::call(buf, outbuf);

    //pc.printf("%s\r\n", outbuf);

    pc.printf("%d\r\n", num);

    //wait(0.1);

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