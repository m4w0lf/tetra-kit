/*
 * Wireshark message builder
 *
 * 2022-02-10  First release
 *
 */
#include "wiremsg.h"

using namespace Tetra;

WireMsg::WireMsg(int port)
{
    // create output destination socket
    struct sockaddr_in addrOutput;
    memset(&addrOutput, 0, sizeof(struct sockaddr_in));
    addrOutput.sin_family = AF_INET;
    addrOutput.sin_port   = htons(port);
    inet_aton("127.0.0.1", &addrOutput.sin_addr);

    m_socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    connect(m_socketFd, (struct sockaddr *) & addrOutput, sizeof(struct sockaddr));
    printf("Wireshark socket 0x%04x on port %d\n", m_socketFd, port);
    if (m_socketFd < 0)
    {
        perror("Couldn't create output socket");
        exit(-1);
    }
}

WireMsg::~WireMsg()
{
    close(m_socketFd);
}


/**
 * @brief Convert TETRA enum channel to GSM number
 *
 */

uint8_t WireMsg::tetraChannelToGsmChannel(const enum MacLogicalChannel channel)
{
    uint8_t ret;

    switch (channel)
    {
    case AACH:
        ret = GSMTAP_TETRA_AACH;
        break;
        // FIXME case BLCH:
        //ret = GSMTAP_TETRA_BLCH;
        //break;
    case BNCH:
        ret = GSMTAP_TETRA_BNCH;
        break;
    case BSCH:
        ret = GSMTAP_TETRA_BSCH;
        break;
    case SCH_F:
        ret = GSMTAP_TETRA_SCH_F;
        break;
    case SCH_HD:
        ret = GSMTAP_TETRA_SCH_HD;
        break;
    // FIXME doesn't exists in downlink case SCH_HU:
    case STCH:
        ret = GSMTAP_TETRA_STCH;
        break;

    case TCH_S:
    case TCH:
        // TODO to check
        ret = GSMTAP_TETRA_TCH_F;
        break;

    default:
        ret = 0;
        break;
    }

    return ret;
}

/**
 * @brief Send message (GSMTAP header + tetra L1 bits) to Wireshark
 *
 */

void WireMsg::sendMsg(const enum MacLogicalChannel tetraChannel, const struct TetraTime tetraTime, Pdu pdu)
{
    struct gsmtapHdr hdr;
    unsigned int hdrLen = sizeof(gsmtapHdr);

    hdr.version      = GSMTAP_VERSION;
    hdr.hdr_len      = hdrLen / 4;                                              // size in 32 bits
    hdr.type         = GSMTAP_TYPE_TETRA_I1;
    hdr.timeslot     = tetraTime.tn;                                            //tup->tdma_time.tn

    hdr.arfcn        = 0;
    hdr.signal_dbm   = 0;                                                       //signal_dbm;
    hdr.snr_db       = 0;                                                       //snr;

    hdr.frame_number = htonl(18 * tetraTime.mn + tetraTime.fn);                 //htonl(fn);

    hdr.sub_type     = tetraChannelToGsmChannel(tetraChannel);                  // tup->lchan
    hdr.antenna_nr   = 0;
    hdr.sub_slot     = 0;                                                       //ss;
    hdr.res          = 0;

    // calculate packed buffer length
    unsigned int packedBitsLen = pdu.size() / 8;
    if (packedBitsLen % 8)
    {
        // round to upper byte
        packedBitsLen++;
    }

    uint8_t * msg = (uint8_t *)calloc(1, hdrLen + packedBitsLen);

    // copy header at msg start
    memcpy(msg, &hdr, hdrLen);

    // pack pdu bites to bytes and append data to message
    pdu.toPackedUInt8(msg + hdrLen);

    // write to socket
    write(m_socketFd, msg, hdrLen + packedBitsLen);

    // free message
    free(msg);
}
