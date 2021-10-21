#include "mm.h"

using namespace Tetra;

/**
 * @brief MM D-MM STATUS - 16.9.2.5
 *
 */

void Mm::parseDMmStatus(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_mm_status", pdu.toString().c_str());

    uint32_t pos = 4;                                                           // pdu type

    uint32_t statusDownlink = pdu.getValue(pos, 6);

    // 16.10.48

    switch (statusDownlink)
    {
    case 0b000001:
        parseDChangeOfEnergySavingModeRequest(pdu);
        break;
    case 0b000010:
        parseDChangeOfEnergySavingModeResponse(pdu);
        break;
    case 0b000011:
        parseDDualWatchModeResponse(pdu);
        break;
    case 0b000100:
        parseDTerminatingDualWatchModeResponse(pdu);
        break;
    case 0b000101:
        parseDChangeOfDualWatchModeRequest(pdu);
        break;
    case 0b000111:
        parseDMsFrequencyBandsRequest(pdu);
        break;
    case 0b001000:
        parseDDistanceReportingRequest(pdu);
        break;
    }
}

/**
 * @brief MM D-CHANGE OF ENERGY SAVING MODE REQUEST - 16.9.2.5.2
 *
 */

void Mm::parseDChangeOfEnergySavingModeRequest(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_change_of_esm_request", pdu.toString().c_str());

    m_report->start("MM", "D-CHANGE OF ENERGY SAVING MODE REQUEST", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    pos = parseEnergySavingInformation(pdu, pos);

    m_report->send();
}

/**
 * @brief MM D-CHANGE OF ENERGY SAVING MODE RESPONSE - 16.9.2.5.3
 *
 */

void Mm::parseDChangeOfEnergySavingModeResponse(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_change_of_esm_response", pdu.toString().c_str());

    m_report->start("MM", "D-CHANGE OF ENERGY SAVING MODE RESPONSE", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    pos = parseEnergySavingInformation(pdu, pos);

    m_report->send();
}

/**
 * @brief MM D-DUAL WATCH MODE RESPONSE - 16.9.2.5.4
 *
 */

void Mm::parseDDualWatchModeResponse(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_dual_watch_mode_response", pdu.toString().c_str());

    m_report->start("MM", "D-DUAL WATCH MODE RESPONSE", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    pos = parseEnergySavingInformation(pdu, pos);

    m_report->add("result of dual watch request", pdu.getValue(pos, 3));
    pos += 3;

    m_report->add("reserved", pdu.getValue(pos, 8));
    pos += 8;

    // type 2
    uint8_t oBit = pdu.getValue(pos, 1);
    pos += 1;

    if (oBit)
    {
        uint8_t pBit = pdu.getValue(pos, 1);
        pos +=1 ;

        if (pBit)
        {
            pos = parseScchInformationAndDistribution(pdu, pos);
        }
    }

    m_report->send();
}

/**
 * @brief MM D-TERMINATING DUAL WATCH MODE RESPONSE - 16.9.2.5.3
 *
 */

void Mm::parseDTerminatingDualWatchModeResponse(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_terminating_dwm_response", pdu.toString().c_str());

    m_report->start("MM", "D-TERMINATING DUAL WATCH MODE RESPONSE", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    m_report->add("reserved", pdu.getValue(pos, 8));
    pos += 8;

    // type 2
    uint8_t oBit = pdu.getValue(pos, 1);
    pos += 1;

    if (oBit)
    {
        uint8_t pBit = pdu.getValue(pos, 1);
        pos += 1;

        if (pBit)
        {
            pos = parseEnergySavingInformation(pdu, pos);
        }

        pBit = pdu.getValue(pos, 1);
        pos += 1;

        if (pBit)
        {
            pos = parseScchInformationAndDistribution(pdu, pos);
        }
    }

    m_report->send();
}


/**
 * @brief MM D-CHANGE OF DUAL WATCH MODE REQUEST - 16.9.2.5.6
 *
 */

void Mm::parseDChangeOfDualWatchModeRequest(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_change_of_dwm_request", pdu.toString().c_str());

    m_report->start("MM", "D-CHANGE OF DUAL WATCH MODE REQUEST", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    pos = parseEnergySavingInformation(pdu, pos);

    m_report->add("reason for dual watch change by swmi", pdu.getValue(pos, 3));
    pos += 3;

    m_report->add("reserved", pdu.getValue(pos, 8));
    pos += 8;

    // type 2
    uint8_t oBit = pdu.getValue(pos, 1);
    pos += 1;

    if (oBit)
    {
        uint8_t pBit = pdu.getValue(pos, 1);
        pos +=1 ;

        if (pBit)
        {
            pos = parseScchInformationAndDistribution(pdu, pos);
        }
    }

    m_report->send();
}


/**
 * @brief MM D-MS FREQUENCY BANDS REQUEST - 16.9.2.5.8
 *
 */

void Mm::parseDMsFrequencyBandsRequest(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_ms_frequency_bands_request", pdu.toString().c_str());

    m_report->start("MM", "D-MS FREQUENCY BANDS REQUEST", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    // proprietary

    m_report->send();
}

/**
 * @brief MM D-CHANGE OF ENERGY SAVING MODE REQUEST - 16.9.2.5.2
 *
 */

void Mm::parseDDistanceReportingRequest(Pdu pdu)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_d_distance_reporting_request", pdu.toString().c_str());

    m_report->start("MM", "D-DISTANCE REPORTING REQUEST", m_tetraTime, m_macAddress);

    uint32_t pos = 4;

    m_report->add("status downlink", pdu.getValue(pos, 6));
    pos += 6;

    m_report->add("distance reporting timer", pdu.getValue(pos, 7));
    pos += 7;

    m_report->add("distance reporting validity", pdu.getValue(pos, 1));
    pos += 1;

    m_report->send();
}
