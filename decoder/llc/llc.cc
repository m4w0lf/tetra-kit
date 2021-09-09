#include "llc.h"

using namespace Tetra;

/**
 * @brief Constructor
 *
 * Note: LLC service MLE layer only
 *
 */

Llc::Llc(Log * log, Report * report, Mle * mle) : Layer(log, report)
{
    m_mle = mle;
}

/**
 * @brief Destructor
 *
 */

Llc::~Llc()
{

}

/**
 * @brief LLC service entry point
 *
 * Process LLC PDU 21.2.1
 *
 */

void Llc::service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress)
{
    // call base class function
    Layer::service(macLogicalChannel, tetraTime, macAddress);
    
     m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_llc", (macLogicalChannelName(macLogicalChannel)).c_str(), pdu.toString().c_str());
       
    if (macLogicalChannel == BSCH)                                              // TM-SDU is directly sent to MLE
    {
        m_mle->service(pdu, macLogicalChannel, m_tetraTime, m_macAddress);
        return;
    }

    std::string txt = "";
    uint8_t advancedLink;
    uint8_t dfinal = -1;
    uint8_t ackLength = 0;

    Pdu sdu;                                                                    // empty SDU

    uint32_t pos = 0;                                                           // current position in pdu stream
    uint8_t pduType = pdu.getValue(pos, 4);                                     // 21.2.1 table 21.1
    pos += 4;

    switch (pduType)
    {
    case 0b0000:                                                                // BL-ADATA
        txt = "BL-ADATA";
        pos += 1;                                                               // nr
        pos += 1;                                                               // ns
        sdu = Pdu(pdu, pos);                                               
        break;

    case 0b0001:                                                                // BL-DATA
        txt = "BL-DATA";
        pos += 1;                                                               // ns
        sdu = Pdu(pdu, pos);
        break;

    case 0b0010:                                                                // BL-UDATA
        txt = "BL-UDATA";
        sdu = Pdu(pdu, pos);
        break;

    case 0b0011:                                                                // BL-ACK
        txt = "BL-ACK";
        pos += 1;                                                               // nr
        sdu = Pdu(pdu, pos);
        break;

    case 0b0100:                                                                // BL-ADATA + FCS
        txt = "BL-ADATA + FCS";
        pos += 1;                                                               // nr
        pos += 1;                                                               // ns
        sdu = Pdu(pdu, pos, (int32_t)pdu.size() - (int32_t)pos  - 32);          // TODO removed FCS for now
        break;

    case 0b0101:                                                                // BL-DATA + FCS
        txt = "BL-DATA + FCS";
        sdu = Pdu(pdu, pos, (int32_t)pdu.size() - (int32_t)pos  - 32);          // TODO removed FCS for now
        break;

    case 0b0110:                                                                // BL-UDATA + FCS
        txt = "BL-UDATA + FCS";
        sdu = Pdu(pdu, pos, (int32_t)pdu.size() - (int32_t)pos - 32);           // TODO removed FCS for now
        break;

    case 0b0111:                                                                // BL-ACK + FCS
        txt = "BL-ACK + FCS";
        pos += 1;                                                               // nr
        sdu = Pdu(pdu, pos, 32);                                                // TODO removed FCS for now
        break;

    case 0b1000:                                                                // AL-SETUP
        txt = "AL-SETUP";
        advancedLink = pdu.getValue(pos, 1);
        pos += 1;
        pos += 2;
        pos += 3;
        pos += 1;
        pos += 1;
        pos += 2;
        pos += 3;
        pos += 2;
        pos += 3;
        pos += 4;
        pos += 3;
        if (advancedLink == 0)
        {
            pos += 8;                                                           // ns
        }
        break;

    case 0b1001:                                                                // AL-DATA/AL-DATA-AR/AL-FINAL/AL-FINAL-AR
        dfinal = pdu.getValue(pos, 1);
        pos += 1;
        if (dfinal)
        {
            txt = "AL-FINAL/AL-FINAL-AR";
        }
        else
        {
            txt = "AL-DATA/AL-DATA-AR";
        }
        pos += 1;                                                               // ar
        pos += 3;                                                               // ns
        pos += 8;                                                               // ss
        sdu = Pdu(pdu, pos);
        break;

    case 0b1010:                                                                // AL-UDATA/AL-UFINAL
        dfinal = pdu.getValue(pos, 1); pos += 1;
        if (dfinal)
        {
            txt = "AL-UDATA";
        }
        else
        {
            txt = "AL-UFINAL";
        }
        pos += 8;                                                               // ns
        pos += 8;                                                               // ss
        sdu = Pdu(pdu, pos);
        break;

    case 0b1011:                                                                // AL-ACK/AL-UNR
        txt = "AL-ACK/AL-UNR";
        pos += 1;                                                               // flow control
        pos += 3;                                                               // nr - table 314 number of tl-sdu
        ackLength = pdu.getValue(pos, 6);
        pos += 6;
        if ((ackLength >= 0b000001) && (ackLength <= 0b111110))
        {
            pos += 8;                                                           // sr
        }
        else
        {

        }
        break;

    case 0b1100:                                                                // AL-RECONNECT
        txt = "AL-RECONNECT";
        break;

    case 0b1101:                                                                // supplementary LLC PDU (table 21.3)
        txt = "supplementary LLC PDU";
        break;

    case 0b1110:                                                                // layer-2 signalling PDU (table 21.2)
        txt = "layer 2 signalling PDU";
        break;

    case 0b1111:                                                                // AL-DISC
        txt = "AL-DISC";
        break;
    }

    m_log->print(LogLevel::LOW, "service_llc : TN/FN/MN = %2u/%2u/%2u  %-20s\n", m_tetraTime.tn, m_tetraTime.fn, m_tetraTime.mn, txt.c_str());

    if (!sdu.isEmpty())                                                         // service MLE
    {
        m_mle->service(sdu, macLogicalChannel, m_tetraTime, m_macAddress);
    }
}
