#include "mle.h"
#include <ctime>

using namespace Tetra;

/**
 * @brief Constructor
 *
 */

Mle::Mle(Log * log, Report * report, Layer * cmce, Layer * mm, Layer * sndcp) : Layer(log, report)
{
    m_cmce = cmce;
    m_mm = mm;
    m_sndcp = sndcp;
}

/**
 * @brief Destructor
 *
 */

Mle::~Mle()
{

}

/**
 * @brief MLE service entry point
 *
 */

void Mle::service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress)
{
    Layer::service(macLogicalChannel, tetraTime, macAddress);

    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_mle", macLogicalChannelName(macLogicalChannel).c_str(), pdu.toString().c_str());

    std::string txt   = "";
    std::string infos = "";
    bool bPrintInfos = false;

    if (macLogicalChannel == BSCH)                                              // TM-SDU was already sent directly by MAC 18.4.2. Report infos and stop here
    {
        bPrintInfos = true;
        txt = "MLE";
        std::stringstream tt;
        tt << "D-MLE-SYNC (MCC=" << pdu.getValue(0, 10) << " MNC=" << pdu.getValue(10, 14) << ")";
        infos = tt.str();
        return;                                                                 // TODO clean up
    }
    else if (macLogicalChannel == BNCH)                                         // TM-SDU was already sent directly by MAC 18.4.2. Report infos and stop here
    {
        bPrintInfos = true;
        txt = "MLE";
        std::stringstream tt;
        tt << "D-MLE-SYSINFO (LA=" << pdu.getValue(0, 14) << " SS=" << pdu.getValue(14, 16) << " BS=" << pdu.getValue(30, 12) << ")";
        infos = tt.str();
        return;                                                                 // TODO clean up
    }
    else                                                                        // use discriminator - see 18.5.21
    {
        uint32_t pos = 0;
        uint8_t disc = pdu.getValue(pos, 3);
        pos += 3;

        switch (disc)
        {
        case 0b000:
            txt = "reserved";
            break;

        case 0b001:                                                             // transparent -> remove discriminator and send directly to MM
            txt = "MM";
            m_mm->service(Pdu(pdu, pos), macLogicalChannel, m_tetraTime, m_macAddress);
            break;

        case 0b010:
            txt = "CMCE";                                                       // transparent -> remove discriminator and send directly to CMCE
            m_cmce->service(Pdu(pdu, pos), macLogicalChannel, m_tetraTime, m_macAddress);
            break;

        case 0b011:
            txt = "reserved";
            break;

        case 0b100:
            txt = "SNDCP";                                                      // transparent -> remove discriminator and send directly to SNDCP
            m_sndcp->service(Pdu(pdu, pos), macLogicalChannel, m_tetraTime, m_macAddress);
            break;

        case 0b101:                                                             // remove discriminator bits and send to MLE sub-system (for clarity only)
            txt = "MLE subsystem";
            serviceMleSubsystem(Pdu(pdu, pos), macLogicalChannel);              // no need to explicitely add tetraTime and macAddress since this is a class private function
            break;

        case 0b110:
        case 0b111:
            txt = "reserved";
            break;
        }
    }

    if (bPrintInfos)
    {
        m_log->print(LogLevel::LOW, "service_mle : TN/FN/MN = %2u/%2u/%2u  %-20s  %-20s\n", m_tetraTime.tn, m_tetraTime.fn, m_tetraTime.mn, txt.c_str(), infos.c_str());
    }
}

/**
 * @brief Service MLE subsystem
 *
 */

void Mle::serviceMleSubsystem(Pdu pdu, MacLogicalChannel macLogicalChannel)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_mle_subsystem", macLogicalChannelName(macLogicalChannel).c_str(), pdu.toString().c_str());

    std::string txt = "";

    uint32_t pos = 0;

    uint8_t pduType = pdu.getValue(pos, 3);
    pos += 3;

    switch (pduType)
    {
    case 0b000:
        txt = "D-NEW-CELL";
        break;

    case 0b001:
        txt = "D-PREPARE-FAIL";
        break;

    case 0b010:
        txt = "D-NWRK-BROADCAST";
        processDNwrkBroadcast(pdu);
        break;

    case 0b011:
        txt = "D-NWRK-BROADCAST-EXTENSION";
        processDNwrkBroadcastExtension(pdu);
        break;

    case 0b100:
        txt = "D-RESTORE-ACK";
        m_cmce->service(Pdu(pdu, pos), macLogicalChannel, m_tetraTime, m_macAddress);
        break;

    case 0b101:
        txt = "D-RESTORE-FAIL";
        break;

    case 0b110:
        txt = "D-CHANNEL-RESPONSE";
        break;

    case 0b111:
        txt = "reserved";
        break;
    }

    m_log->print(LogLevel::LOW, "serv_mle_sub: TN/FN/MN = %2u/%2u/%2u  %-20s\n", m_tetraTime.tn, m_tetraTime.fn, m_tetraTime.mn, txt.c_str());
}

/**
 * @brief Process D-NWRK-BROADCAST 18.4.1.4.1
 *
 */

void Mle::processDNwrkBroadcast(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mle_process_d_nwrk_broadcast", pdu.toString().c_str());

    m_report->start("MLE", "D-NWRK-BROADCAST", m_tetraTime, m_macAddress);

    uint32_t pos = 3;                                                           // PDU type

    m_report->add("cell re-select parameter", pdu.getValue(pos, 16));
    pos += 16;

    m_report->add("cell service level", pdu.getValue(pos, 2));
    pos += 2;

    uint8_t oFlag = pdu.getValue(pos, 1);                                       // option flag
    pos += 1;
    if (oFlag)                                                                  // there is type2 or type3/4 fields
    {
        uint8_t pFlag;                                                          // presence flag
        pFlag = pdu.getValue(pos, 1);
        pos += 1;

        if (pFlag)                                                              // 18.5.24 TETRA network time
        {
            uint32_t utctime = pdu.getValue(pos, 24) * 2;
            pos += 24;

            uint8_t sign = pdu.getValue(pos, 1);
            pos += 1;

            uint8_t looffset = pdu.getValue(pos, 6);
            pos += 6;

            int offsetsec = looffset * (sign ? -15 : 15) * 60;

            uint32_t year = pdu.getValue(pos, 6);
            pos += 6;

            pos += 11;                                                          // reserved

            time_t rawtime = 0;                                                 // 1.1.1900 00:00:00
            struct tm * timeinfo;
            timeinfo = localtime(&rawtime);
            timeinfo->tm_year = 100 + year;
            rawtime = mktime(timeinfo);
            rawtime += utctime + offsetsec;

            m_report->add("tetra network time", ctime(&rawtime));               // ToDo: encode it as ISO8601
        }

        pFlag = pdu.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            uint8_t neighbourCellsCount = pdu.getValue(pos, 3);
            pos += 3;
            m_report->add("number of neighbour cells", neighbourCellsCount);

            for (uint8_t cnt = 0; cnt < neighbourCellsCount; cnt++)
            {
                std::vector<std::tuple<std::string, uint64_t>> infos;

                infos.clear();
                pos = parseNeighbourCellInformation(pdu, pos, infos);

                m_report->addArray(formatStr("cell %u", cnt), infos);
            }
        }
    }

    m_report->send();
}

/**
 * @brief Parse neighbour cell information 18.5.17 and return actual data length read
 *        to increase flux position. This function used by mle_process_d_nwrk_broadcast
 *
 */

uint32_t Mle::parseNeighbourCellInformation(Pdu data, uint32_t posStart, std::vector<std::tuple<std::string, uint64_t>> & infos)
{
    uint32_t pos = posStart;

    infos.push_back(std::make_tuple("identifier", data.getValue(pos, 5)));
    pos += 5;

    infos.push_back(std::make_tuple("reselection types supported", data.getValue(pos, 2)));
    pos += 2;

    infos.push_back(std::make_tuple("neighbour cell synchronized", data.getValue(pos, 1)));
    pos += 1;

    infos.push_back(std::make_tuple("service level", data.getValue(pos, 2)));
    pos += 2;

    infos.push_back(std::make_tuple("main carrier number", data.getValue(pos, 12)));
    pos += 12;

    uint8_t oFlag = data.getValue(pos, 1);                                      // option flag
    pos += 1;
    if (oFlag)                                                                  // there is type2 fields
    {
        uint8_t pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("main carrier number extension", data.getValue(pos, 10)));
            pos += 10;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("MCC", data.getValue(pos, 10)));
            pos += 10;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("MNC", data.getValue(pos, 14)));
            pos += 14;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("LA", data.getValue(pos, 14)));
            pos += 14;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("max. MS tx power", data.getValue(pos, 3)));
            pos += 3;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("min. rx access level", data.getValue(pos, 4)));
            pos += 4;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("subscriber class", data.getValue(pos, 16)));
            pos += 16;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("BS service details", data.getValue(pos, 12)));
            pos += 12;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("timeshare or security", data.getValue(pos, 5)));
            pos += 5;
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("TDMA frame offset", data.getValue(pos, 6)));
            pos += 6;
        }
    }

    return pos;
}

/**
 * @brief Process D-NWRK-BROADCAST-EXTENSION 18.4.1.4.1a
 *
 */

void Mle::processDNwrkBroadcastExtension(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mle_process_d_nwrk_broadcast_extension", pdu.toString().c_str());

    m_report->start("MLE", "D-NWRK-BROADCAST-EXTENSION", m_tetraTime, m_macAddress);

    uint32_t pos = 3;                                                           // PDU type

    uint8_t oFlag = pdu.getValue(pos, 1);                                       // option flag
    pos += 1;
    if (oFlag)                                                                  // there is type2 or type3/4 fields
    {
        uint8_t pFlag;                                                          // presence flag

        pFlag = pdu.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            uint8_t cnt = pdu.getValue(pos, 4);

            m_report->add("number of channel classes", cnt);
            pos += 4;

            // 18.5.5b Channel class
            // TODO parse channel class
        }

    }

    m_report->send();
}
