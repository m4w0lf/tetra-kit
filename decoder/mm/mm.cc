#include "mm.h"

using namespace Tetra;

/**
 * @brief Constructor
 *
 */

Mm::Mm(Log * log, Report * report) : Layer(log, report)
{

}

/**
 * @brief Destructor
 *
 */

Mm::~Mm()
{

}

/**
 * @brief MM service entry point - 16.9
 *
 */

void Mm::service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress)
{
    Layer::service(macLogicalChannel, tetraTime, macAddress);

    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_mm", macLogicalChannelName(macLogicalChannel).c_str(), pdu.toString().c_str());

    std::string txt = "";
    uint32_t pos = 0;
    uint8_t pduType = pdu.getValue(pos, 4);

    // PDU type - 16.10.39 (EN 300 392-2) and A.8.59 (EN 300 392-7)

    switch (pduType)
    {
    case 0b0000:
        txt = "D-OTAR";
        pos += 4;
        parseDOtar(pdu);
        break;

    case 0b0001:
        txt = "D-AUTHENTICATION";
        pos += 4;
        //parseDAuthentication(pdu);
        break;

    case 0b0010:
        txt = "D-CK CHANGE DEMAND";
        pos += 4;
        //parseDCkChangeDemand(pdu);
        break;

    case 0b0011:
        txt = "D-DISABLE";
        //parseDDisable(pdu);
        pos += 4;
        break;

    case 0b0100:
        txt = "D-ENABLE";
        //parseDEnable(pdu);
        pos += 4;
        break;

    case 0b0101:
        txt = "D-LOCATION UPDATE ACCEPT";
        pos += 4;
        //parseDLocationUpdateAccept(pdu);
        break;

    case 0b0110:
        txt = "D-LOCATION UPDATE COMMAND";
        pos += 4;
        //parseDLocationUpdateCommand(pdu);
        break;

    case 0b0111:
        txt = "D-LOCATION UPDATE REJECT";
        pos += 4;
        //parseDLocationUpdateReject(pdu);
        break;

    case 0b1001:
        txt = "D-LOCATION UPDATE PROCEEDING";
        //parseDLocationUpdateProceeding(pdu);
        pos += 4;
        break;

    case 0b1010:
        txt = "D-ATTACH/DETACH GROUP IDENTITY";
        //parseDAttachDetachGroupIdentity(pdu);
        pos += 4;
        break;

    case 0b1011:
        txt = "D-ATTACH/DETACH GROUP IDENTITY ACK";
        //parseDAttachDetachGroupIdentityAck(pdu);
        pos += 4;
        break;

    case 0b1100:
        txt = "D-MM STATUS";
        //parseDMmStatus(pdu);
        pos += 4;
        break;

    case 0b1111:
        txt = "MM PDU/FUNCTION NOT SUPPORTED";
        break;

    default:
        txt = "reserved";
        break;
    }
    
    printf("serv_mm_sub : TN/FN/MN = %2u/%2u/%2u  %-20s  len=%3lu \n", m_tetraTime.tn, m_tetraTime.fn, m_tetraTime.mn, txt.c_str(), pdu.size());
}

/**
 * @brief MM
 *
 */

void Mm::parseXXX(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_XXX", pdu.toString().c_str());

    m_report->start("MM", "XXX", m_tetraTime, m_macAddress);

    m_report->send();
}

std::string Mm::valueToString(std::string key, uint32_t val)
{
    std::string valueAsString = "";

    if (strEqualsU(key, "location update accept type"))
    {
        switch (val)
        {
        case 0b000:
            valueAsString = "Roaming location updating";
            break;
        case 0b001:
            valueAsString = "Temporary registration";
            break;
        }
    }
    if (strEqualsU(key, "energy saving mode"))
    {
        switch (val)
        {
        case 0b000:
            valueAsString = "Stay Alive";
            break;
        }
    }
    return valueAsString;
}


