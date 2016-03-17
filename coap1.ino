/*
* WARNING - UDP_TX_PACKET_MAX_SIZE is hardcoded by Arduino to 24 bytes
* This limits the size of possible outbound UDP packets
*/

#include <SPI.h>
#include <Ethernet2.h>
#include <stdint.h>
#include <EthernetUdp2.h>
#include "coap.h"

#define PORT 5683
static uint8_t mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
IPAddress ip(192, 168, 1, 1);

EthernetClient client;
EthernetUDP udp;
uint8_t packetbuf[256];
static uint8_t scratch_raw[32];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};

void setup()
{
    int i;
    Serial.begin(9600);
    while (!Serial) 
    {
        ; // wait for serial port to connect. Needed for Leonardo only
    }

    // start the Ethernet connection:
    Ethernet.begin(mac, ip);
    Serial.print("My IP address: ");
    for (i=0; i<4; i++)
    {
        Serial.print(Ethernet.localIP()[i], DEC);
        Serial.print("."); 
    }
    Serial.println();
    udp.begin(PORT);

    coap_setup();
    endpoint_setup();
}

void udp_send(const uint8_t *buf, int buflen)
{
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    while(buflen--)
        udp.write(*buf++);
    udp.endPacket();
}

void loop()
{
    int taille;
    int rc;
    coap_packet_t packet;
    int i;
    
    if ((taille = udp.parsePacket()) > 0)
    {
        udp.read(packetbuf, sizeof(packetbuf));

        for (i=0; i<taille; i++)
        {
            Serial.print(packetbuf[i], HEX);
            Serial.print(" ");
        }
        Serial.println("");

        if (0 != (rc = coap_parse(&packet, packetbuf, taille)))
        {
            Serial.print("Bad packet rc=");
            Serial.println(rc, DEC);
        }
        else
        {
            size_t rsplen = sizeof(packetbuf);
            coap_packet_t received_packet;
            coap_handle_req(&scratch_buf, &packet, &received_packet);

            memset(packetbuf, 0, UDP_TX_PACKET_MAX_SIZE);
            if (0 != (rc = coap_build(packetbuf, &rsplen, &received_packet)))
            {
                Serial.print("coap_build failed rc=");
                Serial.println(rc, DEC);
            }
            else
            {
                Serial.print("RESPONSE ");
                Serial.print("code = "); Serial.print(received_packet.hdr.tkl);
                Serial.print(" numopts = "); Serial.print(received_packet.numopts, DEC); Serial.print(" [");
                for(i = 0; i < received_packet.numopts; i++) {
                    Serial.print("#");
                    Serial.print(received_packet.opts[i].num, DEC);
                    Serial.print("(");
                    Serial.write((const char *)received_packet.opts[i].buf.p, received_packet.opts[i].buf.len);
                    Serial.print(")");
                }
                Serial.print("] payload = "); Serial.write((const char *)received_packet.payload.p, received_packet.payload.len);
                Serial.println("");
                udp_send(packetbuf, rsplen);
            }
        }
    }
}


