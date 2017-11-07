#include "newcoap.h"
#define LOGGING
NewCoap::NewCoap() {
}
uint16_t NewCoap::buildPacket(NewCoapPacket &packet, uint8_t *buf,uint16_t& packetSize) {
   
    uint8_t *p = buf;
    uint16_t running_delta = 0;
    packetSize = 0;
    // make coap packet base header
    *p = 0x01 << 6;
    *p |= (packet.type & 0x03) << 4;
    *p++ |= (packet.tokenlen & 0x0F);
    *p++ = packet.code;
    *p++ = (packet.messageid >> 8);
    *p++ = (packet.messageid & 0xFF);
    p = buf + COAP_HEADER_SIZE;
    packetSize += 4;
    // make token
    if (packet.token != NULL && packet.tokenlen <= 0x0F) {
        memcpy(p, packet.token, packet.tokenlen);
        p += packet.tokenlen;
        packetSize += packet.tokenlen;
    }
    // make option header
    for (int i = 0; i < packet.optionnum; i++)  {
        uint32_t optdelta;
        uint8_t len, delta;
        if (packetSize + 5 + packet.options[i].length >= BUF_MAX_SIZE) {
            return 0;
        }
        optdelta = packet.options[i].number - running_delta;
        COAP_OPTION_DELTA(optdelta, &delta);
        COAP_OPTION_DELTA((uint32_t)packet.options[i].length, &len);
        *p++ = (0xFF & (delta << 4 | len));
        if (delta == 13) {
            *p++ = (optdelta - 13);
            packetSize++;
        } else if (delta == 14) {
            *p++ = ((optdelta - 269) >> 8);
            *p++ = (0xFF & (optdelta - 269));
            packetSize+=2;
        } if (len == 13) {
            *p++ = (packet.options[i].length - 13);
            packetSize++;
        } else if (len == 14) {
            *p++ = (packet.options[i].length >> 8);
            *p++ = (0xFF & (packet.options[i].length - 269));
            packetSize+=2;
        }
        memcpy(p, packet.options[i].buf, packet.options[i].length);
        p += packet.options[i].length;
        packetSize += packet.options[i].length + 1;
        running_delta = packet.options[i].number;
    }
    // make payload
    if (packet.payloadlen > 0) {
        if ((packetSize + 1 + packet.payloadlen) >= BUF_MAX_SIZE) {
            return 0;
        }
        *p++ = 0xFF;
        memcpy(p, packet.payload, packet.payloadlen);
        packetSize += 1 + packet.payloadlen;
    }
        return 1;
   
}
uint16_t NewCoap::build(char *url, COAP_TYPE type, COAP_METHOD method, uint8_t *token, uint8_t tokenlen, uint8_t *payload, uint32_t payloadlen,uint8_t *buf,uint16_t &packetSize) {
    // make packet
    NewCoapPacket packet;
    packet.type = type;
    packet.code = method;
    packet.token = token;
    packet.tokenlen = tokenlen;
    packet.payload = payload;
    packet.payloadlen = payloadlen;
    packet.optionnum = 0;
    packet.messageid = rand();
    
      packet.options[packet.optionnum].buf = (uint8_t *)url;
    packet.options[packet.optionnum].length = strlen(url);
    packet.options[packet.optionnum].number = COAP_URI_PATH;
      packet.optionnum++;
    
    // build packet
    return this->buildPacket(packet,buf,packetSize);
}
