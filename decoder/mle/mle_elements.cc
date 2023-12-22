#include "mle.h"

using namespace Tetra;

/**
 * @brief 18.5.17 Neighbour cell information for CA
 *
*/

uint64_t Mle::parseBsServiceDetails(Pdu pdu, uint64_t pos, std::vector<std::tuple<std::string, uint64_t>> & infos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mle_parse_bs_service_details", pdu.toString().c_str());

    infos.push_back(std::make_tuple("registration", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("de-registration", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("priority cell", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("minimum mode service", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("migration", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("system wide services", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("tetra voice service", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("circuit mode data service", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("reserved", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("sndcp service", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("air interface encryption service", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("advanced link supported", pdu.getValue(pos, 1)));
    pos += 1;

    return pos;
}

uint64_t Mle::parseNeighbourCellBroadcast(Pdu pdu, uint64_t pos, std::vector<std::tuple<std::string, uint64_t>> & infos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mle_parse_neighbour_cell_broadcast", pdu.toString().c_str());

    infos.push_back(std::make_tuple("D-NWRK-BROADCAST broadcast supported", pdu.getValue(pos, 1)));
    pos += 1;
    infos.push_back(std::make_tuple("D-NWRK-BROADCAST enquiry supported", pdu.getValue(pos, 1)));
    pos += 1;

    return pos;
}


/**
 * @brief Parse neighbour cell information 18.5.17 and return actual data length read
 *        to increase flux position. This function used by mle_process_d_nwrk_broadcast
 *
 */

uint32_t Mle::parseNeighbourCellInformation(Pdu data, uint32_t posStart, std::vector<std::tuple<std::string, uint64_t>> & infos)
{
    uint32_t pos = posStart;

    infos.push_back(std::make_tuple("Cell identifier CA", data.getValue(pos, 5)));
    pos += 5;

    infos.push_back(std::make_tuple("Cell reselection types supported", data.getValue(pos, 2)));
    pos += 2;

    infos.push_back(std::make_tuple("Neighbour cell synchronized", data.getValue(pos, 1)));
    pos += 1;

    infos.push_back(std::make_tuple("Cell load CA", data.getValue(pos, 2)));
    pos += 2;

    infos.push_back(std::make_tuple("Main carrier number", data.getValue(pos, 12)));
    pos += 12;

    uint8_t oFlag = data.getValue(pos, 1);                                      // option flag
    pos += 1;
    if (oFlag)                                                                  // there is type2 fields
    {
        uint8_t pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            infos.push_back(std::make_tuple("Main carrier number extension", data.getValue(pos, 10)));
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
            pos = parseBsServiceDetails(data, pos, infos);
        }

        pFlag = data.getValue(pos, 1);
        pos += 1;
        if (pFlag)
        {
            pos = parseTimeshareOrSecurity(data, pos, infos);
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
 * @brief 18.5.25 Timeshare cell information or security parameters
 *
*/

uint64_t Mle::parseTimeshareOrSecurity(Pdu pdu, uint64_t pos, std::vector<std::tuple<std::string, uint64_t>> & elements)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mle_parse_timeshare_or_security", pdu.toString().c_str());

    uint8_t discMode = pdu.getValue(pos, 2);
    elements.push_back(std::make_tuple("Discontinuous mode", pdu.getValue(pos, 2)));
    pos += 2;

    if (discMode == 0)
    {
        pos = parseSecurityParameters(pdu, pos, elements);
    }
    else
    {
        elements.push_back(std::make_tuple("Reserved frames per two multiframes", pdu.getValue(pos, 3)));
        pos += 3;
    }

    return pos;
}

/**
 * @brief A.8.77a Security parameters
 *
*/

uint64_t Mle::parseSecurityParameters(Pdu pdu, uint64_t pos, std::vector<std::tuple<std::string, uint64_t>> & elements)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mle_parse_security_parameters", pdu.toString().c_str());

    elements.push_back(std::make_tuple("Authentication", pdu.getValue(pos, 1)));
    pos += 1;

    elements.push_back(std::make_tuple("Security Class 1", pdu.getValue(pos, 1)));
    pos += 1;

    elements.push_back(std::make_tuple("Security Class 2 or 3", pdu.getValue(pos, 1)));
    pos += 1;

    return pos;
}
