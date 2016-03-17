#include <stdbool.h>
#include <string.h>
#include <Arduino.h>
#include "coap.h"
#include "dht.h"

const uint16_t rsplen = 1500;
static char rsp[1500] = "";
#define DHT22PIN A1
dht DHT;

// HANDLERS

static const coap_endpoint_path_t PATH_well_known_core = {2, {".well-known", "core"}};
static int handle_GET_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static const coap_endpoint_path_t PATH_temperature = {1, {"temperature"}};
static int handle_GET_temperature(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    char response[16];
    
    int chk = DHT.read22(DHT22PIN);
    switch (chk)
    {
      case DHTLIB_OK:
          // 1H30 de travail : pour trouver la seule fonction du monde en AVR capable de transormer un double en string
          dtostrf(DHT.temperature, 5, 2, response);
          break;
      case DHTLIB_ERROR_CHECKSUM:
          strncpy(response, "Checksum error", 16);
          break;
      case DHTLIB_ERROR_TIMEOUT:
          strncpy(response, "Time out error", 16);
          break;
      case DHTLIB_ERROR_CONNECT:
          strncpy(response, "Connect error", 16);
          break;
      case DHTLIB_ERROR_ACK_L:
          strncpy(response, "Ack Low error", 16);
          break;
      case DHTLIB_ERROR_ACK_H:
          strncpy(response, "Ack High error", 16);
          break;
      default:
          strncpy(response, "Unknown error", 16);
          break; 
    }
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(response), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static const coap_endpoint_path_t PATH_humidity = {1, {"humidity"}};
static int handle_GET_humidity(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    char response[16];
    
    int chk = DHT.read22(DHT22PIN);
    switch (chk)
    {
      case DHTLIB_OK:
          // 1H30 de travail : pour trouver la seule fonction du monde en AVR capable de transormer un double en string
          dtostrf(DHT.humidity, 5, 2, response);
          break;
      case DHTLIB_ERROR_CHECKSUM:
          strncpy(response, "Checksum error", 16);
          break;
      case DHTLIB_ERROR_TIMEOUT:
          strncpy(response, "Time out error", 16);
          break;
      case DHTLIB_ERROR_CONNECT:
          strncpy(response, "Connect error", 16);
          break;
      case DHTLIB_ERROR_ACK_L:
          strncpy(response, "Ack Low error", 16);
          break;
      case DHTLIB_ERROR_ACK_H:
          strncpy(response, "Ack High error", 16);
          break;
      default:
          strncpy(response, "Unknown error", 16);
          break; 
    }
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(response), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static const coap_endpoint_path_t PATH_light = {1, {"light"}};
static int handle_GET_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    char response[16];
    dtostrf(analogRead(A0)*100.0/1023.0, 5, 2, response); 
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(response), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

// CONFIGURATION DU ENDPOINT

extern const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_GET_well_known_core, &PATH_well_known_core, "ct=40"},
    
    {COAP_METHOD_GET, handle_GET_temperature,     &PATH_temperature,     "ct=0"},
    {COAP_METHOD_GET, handle_GET_humidity,        &PATH_humidity,        "ct=0"},
    {COAP_METHOD_GET, handle_GET_light,           &PATH_light,           "ct=0"},
    
    {(coap_method_t)0, NULL,                      NULL,                  NULL}
};

void build_rsp(void)
{
    uint16_t len = rsplen;
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    while(NULL != ep->handler)
    {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
}

void endpoint_setup(void)
{                
    build_rsp();
}

