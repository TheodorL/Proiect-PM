#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#define IS_SET(n,x)   (((n & (1 << x)) != 0) ? 1 : 0)
#define MULTIBIT_SET(addr, mask, value) (((addr)&(~(mask)))|((value)&(mask)))
#define SET_BIT(n, x) n |= (1 << x)
#define RESET_BIT(n, x) n &= ~(1 << x)

//output register
#define PODR(X) X + 16
//direction register
#define PDR(X) X
//input register
#define PIDR(X) X

//Mux select&signal pins
#define MP_S0 4    //P104 
#define MP_S1 5    //P105
#define MP_S2 6    //P106
#define MP_S3 7    //P107
#define MP_SIG 12  //P112

#define MP_NUM_CH 16

#define NUM_NOTES 12
#define ARP_SIZE 8
#define NO_ARP_POS NUM_NOTES
#define NO_NOTE NUM_NOTES

const uint32_t MP_CH_MASK = (1 << PODR(MP_S0)) | (1 << PODR(MP_S1) ) |
                            (1 << PODR(MP_S2)) | (1 << PODR(MP_S3));
const uint32_t MP_CHANNELS[MP_NUM_CH] = {
    0,
    (1 << PODR(MP_S0)),
    (1 << PODR(MP_S1)),
    (1 << PODR(MP_S1)) | (1 << PODR(MP_S0)),
    (1 << PODR(MP_S2)),
    (1 << PODR(MP_S2)) | (1 << PODR(MP_S0)),
    (1 << PODR(MP_S2)) | (1 << PODR(MP_S1)),
    (1 << PODR(MP_S2)) | (1 << PODR(MP_S1)) | (1 << PODR(MP_S0)),
    (1 << PODR(MP_S3)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S0)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S1)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S1)) | (1 << PODR(MP_S0)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S2)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S2)) | (1 << PODR(MP_S0)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S2)) | (1 << PODR(MP_S1)),
    (1 << PODR(MP_S3)) | (1 << PODR(MP_S2)) | (1 << PODR(MP_S1)) | (1 << PODR(MP_S0)),
};

const uint32_t KEY_NOTES[NUM_NOTES] {
    NOTE_GS6,
    NOTE_G6,
    NOTE_FS6,
    NOTE_F6,
    NOTE_E6,
    NOTE_DS6,
    NOTE_D6,
    NOTE_CS6,
    NOTE_C6,
    NOTE_B5,
    NOTE_AS5,
    NOTE_A5
};


char arpPos[NUM_NOTES];
char arp[ARP_SIZE];
char arpLen = 0;
char arpIdx = 0;



void addArpNote(int noteIdx) {
    if (arpPos[noteIdx] != NO_ARP_POS || arpLen >= ARP_SIZE) return;
    arpPos[noteIdx] = arpLen;
    arp[arpLen++] = noteIdx;
}

void delArpNote(int noteIdx) {
    if (arpPos[noteIdx] == NO_ARP_POS) return;
    if (arpLen <= 0) {
        Serial.println("ERROR");
    }
    int idx = arpPos[noteIdx];
    for (int i = 0; i < NUM_NOTES; i++) {
        if (arpPos[i] > arpPos[noteIdx] && arpPos[i] != NO_ARP_POS){
            arpPos[i]--;
        }
    }
    arpPos[noteIdx] = NO_ARP_POS;
    memmove(arp + idx, arp + idx + 1, arpLen - (idx + 1));
    arpLen--;
}

#endif