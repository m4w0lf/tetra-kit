#ifndef WIREMSG_H
#define WIREMSG_H
#include "../common/tetra.h"
#include "../common/pdu.h"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Tetra {

// from libosmocore/include/core/gsmtap.h

#define GSMTAP_VERSION              0x02
#define GSMTAP_TYPE_TETRA_I1        0x05    /* tetra air interface */
#define GSMTAP_TYPE_TETRA_I1_BURST  0x06    /* tetra air interface */

#define GSMTAP_TETRA_BSCH   0x01
#define GSMTAP_TETRA_AACH   0x02
#define GSMTAP_TETRA_SCH_HU 0x03
#define GSMTAP_TETRA_SCH_HD 0x04
#define GSMTAP_TETRA_SCH_F  0x05
#define GSMTAP_TETRA_BNCH   0x06
#define GSMTAP_TETRA_STCH   0x07
#define GSMTAP_TETRA_TCH_F  0x08
// #define GSMTAP_TETRA_DMO_SCH_S   0x09
// #define GSMTAP_TETRA_DMO_SCH_H   0x0a
// #define GSMTAP_TETRA_DMO_SCH_F   0x0b
// #define GSMTAP_TETRA_DMO_STCH    0x0c
// #define GSMTAP_TETRA_DMO_TCH 0x0d

    ///< Structure of the GSMTAP pseudo-header
    struct gsmtapHdr {
        uint8_t version;                                                        ///< version, set to 0x01 currently
        uint8_t hdr_len;                                                        ///< length in number of 32bit words
        uint8_t type;                                                           ///< see GSMTAP_TYPE_*
        uint8_t timeslot;                                                       ///< timeslot (0..7 on Um)

        uint16_t arfcn;                                                         ///< ARFCN (frequency)
        int8_t signal_dbm;                                                      ///< signal level in dBm
        int8_t snr_db;                                                          ///< signal/noise ratio in dB

        uint32_t frame_number;                                                  ///< GSM Frame Number (FN)

        uint8_t sub_type;                                                       ///< Type of burst/channel, see above
        uint8_t antenna_nr;                                                     ///< Antenna Number
        uint8_t sub_slot;                                                       ///< sub-slot within timeslot
        uint8_t res;                                                            ///< reserved for future use (RFU)
    } __attribute__((packed));

    /**
     * @brief Wireshark messager
     *
     */

    class WireMsg {
    public:
        WireMsg(int port = 4729);
        ~WireMsg();

        uint8_t tetraChannelToGsmChannel(const enum MacLogicalChannel channel);
        void sendMsg(const enum MacLogicalChannel tetraChannel, const struct TetraTime tetraTime, Pdu pdu);
    private:
        int m_socketFd;
    };

};

#endif /* WIREMSG_H */
