#include "mm.h"

using namespace Tetra;

/**
 * @brief MM type 3/4 elements - E.1.1 (see E.1.3 for example)
 *
 */

uint64_t Mm::parseType34Elements(Pdu pdu, uint64_t pos)
{
    while (pdu.getValue(pos, 1))                                                // repeat for all type 3/4 elements
    {
        pos += 1;
        uint64_t elementId = pdu.getValue(pos, 4);                              // type 3/4 element identifier
        uint64_t elementType = 0;

        // 16.10.51 Table 16.89

        switch (elementId)
        {
        case 0b0000:                                                            // Reserved for future extension
            break;
        case 0b0001:                                                            // Default group attachment lifetime
            elementType = 3;                                                    // According to Table 16.11
            break;
        case 0b0010:                                                            // New registered area
            elementType = 4;                                                    // According to Table 16.11
            break;
        case 0b0011:                                                            // Group identity location demand
            break;
        case 0b0100:                                                            // Group report response
            elementType = 3;                                                    // According to Table 16.1
            break;
        case 0b0101:                                                            // Group identity location accept
            elementType = 3;                                                    // According to Table 16.11
            break;
        case 0b0110:                                                            // DM-MS address
            break;
        case 0b0111:                                                            // Group identity downlink
            elementType = 4;                                                    // According to Table 16.1
            break;
        case 0b1000:                                                            // Group identity uplink
            break;
        case 0b1001:                                                            // Authentication uplink
            break;
        case 0b1010:                                                            // Authentication downlink
            elementType = 3;                                                    // According to Table 16.11
            break;
        case 0b1011:                                                            // Extended capabilities
            break;
        case 0b1100:                                                            // Group Identity Security Related Information
            elementType = 4;                                                    // According to Table 16.11
            break;
        case 0b1101:                                                            // reserved
            break;
        case 0b1110:                                                            // reserved
            break;
        case 0b1111:                                                            // Proprietary
            elementType = 3;                                                    // According to Table 16.1
            break;
        }

        std::string txt = valueToString("type 3/4 element identifier", pdu.getValue(pos, 4));
        m_report->add("type 3/4 element identifier", pdu.getValue(pos, 4));
        m_report->add("type 3/4 element identifier val", txt);
        pos += 4;

        m_report->add("length indicator", pdu.getValue(pos, 11));
        pos += 11;

        // type 4 elements may have repeated elements, whereas type 3 elements won't

        if (elementType == 4)
        {
            uint64_t numberOfElements = pdu.getValue(pos, 6);
            pos += 6;

            for (uint8_t cnt = 1; cnt <= numberOfElements; cnt++)
            {
                switch (elementId)
                {
                case 0b0010:
                    pos = parseNewRegisteredArea(pdu, pos);
                    break;
                case 0b0111:
                    pos = parseGroupIdentityDownlink(pdu, pos);
                    break;
                case 0b1100:
                    pos = parseGISRI(pdu, pos);
                    break;
                }
            }
        }

        if (elementType == 3)
        {
            switch (elementId)
            {
            case 0b0001:
                m_report->add("default group attachment lifetime", pdu.getValue(pos, 2));
                pos += 2;
                break;
            case 0b0100:
                pos = parseGroupReportResponse(pdu, pos);
                break;
            case 0b0101:
                pos = parseGroupIdentityLocationAccept(pdu, pos);
                break;
            case 0b1010:
                pos = parseAuthenticationDownlink(pdu, pos);
                break;
            }

            uint8_t oBit = pdu.getValue(pos, 1);                                // o-bit
            pos += 1;

            if (oBit == 0)
            {
                break;                                                          // m-bit won't be present
            }
        }
    }
    return pos;
}

/**
 * @brief MM Address extension - 16.10.1
 *
 */

uint64_t Mm::parseAddressExtension(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_address_extension", pdu.toString().c_str());

    m_report->add("mcc", pdu.getValue(pos, 10));
    pos += 10;
    m_report->add("mnc", pdu.getValue(pos, 14));
    pos += 14;

    return pos;
}

/**
 * @brief MM Authentication downlink - A.7.1
 *
 */

uint64_t Mm::parseAuthenticationDownlink(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_authentication_downlink", pdu.toString().c_str());

    m_report->add("authentication result", pdu.getValue(pos, 1));
    pos += 1;

    m_report->add("tei request flag", pdu.getValue(pos, 1));
    pos += 1;

    uint8_t ckProvisionFlag = pdu.getValue(pos, 1);
    m_report->add("ck provision flag", pdu.getValue(pos, 1));
    pos += 1;

    if (ckProvisionFlag)
    {
        pos = parseCkProvisioningInformation(pdu, pos);
    }
    return pos;
}

/**
 * @brief CCK information - A.8.8 (EN 300 392-7)
 *
 */

uint64_t Mm::parseCckInformation(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_XXX", pdu.toString().c_str());

    m_report->add("cck identifier", pdu.getValue(pos, 16));
    pos += 16;
    m_report->add("key type flag", pdu.getValue(pos, 1));
    pos += 1;
    m_report->add("sealed cck", pdu.getValue(pos, 120));
    pos += 120;
    m_report->add("cck location area information", pdu.getValue(pos, 2));
    pos += 2;

    uint8_t futureKeyFlag = pdu.getValue(pos, 1);
    m_report->add("future key flag", pdu.getValue(pos, 1));
    pos += 1;

    if (futureKeyFlag)
    {
        m_report->add("sealed cck", pdu.getValue(pos, 120));
        pos += 120;
    }
    return pos;
}

/**
 * @brief Ciphering parameters - A.8.12 (EN 300 392-7)
 *
 */

uint64_t Mm::parseCipheringParameters(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_ciphering_parameters", pdu.toString().c_str());

    m_report->add("ksg number", pdu.getValue(pos, 4));
    pos += 4;

    uint8_t securityClass = pdu.getValue(pos, 1);
    m_report->add("security class", pdu.getValue(pos, 1));
    pos += 1;

    if (securityClass)                                                          // Class 3
    {
        m_report->add("tm-sck otar", pdu.getValue(pos, 1));
        pos += 1;
        m_report->add("sdmo and dm-sck otar", pdu.getValue(pos, 1));
        pos += 1;
        m_report->add("gck encryption/otar", pdu.getValue(pos, 1));
        pos += 1;
        m_report->add("security information protocol support", pdu.getValue(pos, 1));
        pos += 1;
    }
    else                                                                        // Class 2
    {
        m_report->add("sck number", pdu.getValue(pos, 5));
        pos += 5;
    }

    return pos;
}

/**
 * @brief CK provisioning information - A.8.14 (EN 300 392-7)
 *
 */

uint64_t Mm::parseCkProvisioningInformation(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_ck_provisioning_information", pdu.toString().c_str());

    uint8_t sckProvisionFlag = pdu.getValue(pos, 1);
    m_report->add("sck provision flag", pdu.getValue(pos, 1));
    pos += 1;

    if (sckProvisionFlag)
    {
        pos = parseSckInformation(pdu, pos);
    }

    uint8_t cckProvisionFlag = pdu.getValue(pos, 1);
    m_report->add("cck provision flag", pdu.getValue(pos, 1));
    pos += 1;

    if (cckProvisionFlag)
    {
        pos = parseCckInformation(pdu, pos);
    }
    return pos;
}

/**
 * @brief MM Energy saving information - 16.10.10
 *
 */

uint64_t Mm::parseEnergySavingInformation(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_energy_saving_information", pdu.toString().c_str());

    std::string txt = valueToString("energy saving mode", pdu.getValue(pos, 3));
    m_report->add("energy saving mode", pdu.getValue(pos, 3));
    m_report->add("energy saving mode val", txt);
    pos += 3;

    m_report->add("frame number", pdu.getValue(pos, 5));
    pos += 5;
    m_report->add("multiframe number", pdu.getValue(pos, 6));
    pos += 6;

    return pos;
}

/**
 * @brief GCK rejected - A.8.28b (EN 300 392-7)
 *
 */

uint64_t Mm::parseGckRejected(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_XXX", pdu.toString().c_str());

    m_report->add("otar reject reason", pdu.getValue(pos, 3));
    pos += 3;

    uint8_t groupAssociation = pdu.getValue(pos, 1);
    m_report->add("group association", pdu.getValue(pos, 1));
    pos += 1;

    if (groupAssociation)                                                   // Associated with specific GSSI
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
    }
    else
    {
        m_report->add("gckn", pdu.getValue(pos, 16));
        pos += 16;
    }

    return pos;
}

/**
 * @brief MM Group identity downlink - 16.10.22
 *
 */

uint64_t Mm::parseGroupIdentityDownlink(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_group_identity_downlink", pdu.toString().c_str());

    uint8_t attachDetachType = pdu.getValue(pos, 1);
    m_report->add("group identity attach/detach type identifier", pdu.getValue(pos, 1));
    pos += 1;

    if (attachDetachType)                                       // detachment
    {
        m_report->add("group identity detachment downlink", pdu.getValue(pos, 2));
        pos += 2;
    }
    else                                                        // attachment
    {
        m_report->add("group identity attachment lifetime", pdu.getValue(pos, 2));
        pos += 2;
        m_report->add("class of usage", pdu.getValue(pos, 3));
        pos += 3;
    }

    uint32_t groupIdentityAddressType = pdu.getValue(pos, 2);
    m_report->add("group identity address type", pdu.getValue(pos, 2));
    pos += 2;

    if (groupIdentityAddressType == 0)                          // GSSI
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
    }
    if (groupIdentityAddressType == 1)                          // GTSI
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
        pos = parseAddressExtension(pdu, pos);
    }
    if (groupIdentityAddressType == 2)                          // (V)GSSI
    {
        m_report->add("(v)gssi", pdu.getValue(pos, 24));
        pos += 24;
    }
    if (groupIdentityAddressType == 3)                          // GTSI+(V)GSSI
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
        pos = parseAddressExtension(pdu, pos);
        m_report->add("(v)gssi", pdu.getValue(pos, 24));
        pos += 24;
    }
    return pos;
}

/**
 * @brief MM Group identity location accept - 16.10.23
 *
 */

uint64_t Mm::parseGroupIdentityLocationAccept(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_group_identity_location_accept", pdu.toString().c_str());

    m_report->add("group identity accept/reject", pdu.getValue(pos, 1));
    pos += 1;

    m_report->add("reserved", pdu.getValue(pos, 1));
    pos += 1;

    uint8_t oBit = pdu.getValue(pos, 1);
    pos += 1;

    // Group Identity Downlink is type 4 element
    if (oBit)
    {
        pos = parseType34Elements(pdu, pos);
    }

    return pos;
}

/**
 * @brief Group Identity Security Related Information - A.8.31a (EN 300 392-7)
 *
 */

uint64_t Mm::parseGISRI(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_gisri", pdu.toString().c_str());

    uint64_t numberOfGroups = pdu.getValue(pos, 5);
    m_report->add("number of groups", pdu.getValue(pos, 5));
    pos += 5;

    for (uint8_t gisri_cnt = 1; gisri_cnt <= numberOfGroups; gisri_cnt++)
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
    }

    uint8_t gckAssociation = pdu.getValue(pos, 1);
    m_report->add("gck association", pdu.getValue(pos, 1));
    pos += 1;

    if (gckAssociation)                                         // GCK association information provided
    {
        m_report->add("gck select number", pdu.getValue(pos, 17));
        pos += 17;
    }

    uint8_t sckAssociation = pdu.getValue(pos, 1);
    m_report->add("sck association", pdu.getValue(pos, 1));
    pos += 1;

    if (sckAssociation)                                         // SCK association information provided
    {
        m_report->add("sck subset grouping type", pdu.getValue(pos, 4));
        pos += 4;
        m_report->add("sck subset number", pdu.getValue(pos, 5));
        pos += 5;
    }
    return pos;
}


/**
 * @brief MM Group report response - 16.10.27a
 *
 */

uint64_t Mm::parseGroupReportResponse(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_group_report_response", pdu.toString().c_str());

    m_report->add("group report response", pdu.getValue(pos, 1));
    pos += 1;

    return pos;
}

/**
 * @brief MM New registered area - 16.10.40
 *
 */

uint64_t Mm::parseNewRegisteredArea(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_new_registered_area", pdu.toString().c_str());

    std::string txt = valueToString("LA timer", pdu.getValue(pos, 3));
    m_report->add("LA timer", pdu.getValue(pos, 3));
    m_report->add("LA timer val", txt);
    pos += 3;

    m_report->add("LA", pdu.getValue(pos, 14));
    pos += 14;

    return pos;
}

/**
 * @brief MM SCCH information and distribution on 18th frame - 16.10.46
 *
 */

uint64_t Mm::parseScchInformationAndDistribution(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_scch_information_and_distribution", pdu.toString().c_str());

    m_report->add("scch information", pdu.getValue(pos, 4));
    pos += 4;

    m_report->add("distribution on 18th frame", pdu.getValue(pos, 2));
    pos += 2;

    return pos;
}

/**
 * @brief SCK information - A.8.68 (EN 300 392-7)
 *
 */

uint64_t Mm::parseSckInformation(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_XXX", pdu.toString().c_str());

    // A.8.68 SCK information

    uint8_t sessionKey = pdu.getValue(pos, 1);
    m_report->add("session key", pdu.getValue(pos, 1));
    pos += 1;

    if (sessionKey)                                                     // session key for group
    {
        m_report->add("gsko-vn", pdu.getValue(pos, 16));
        pos += 16;
    }
    else                                                                // session key for individual
    {
        m_report->add("random seed for otar", pdu.getValue(pos, 80));
        pos += 80;
    }

    m_report->add("sck number", pdu.getValue(pos, 5));
    pos += 5;
    m_report->add("sck version number", pdu.getValue(pos, 16));
    pos += 16;
    m_report->add("sealed sck", pdu.getValue(pos, 120));
    pos += 120;

    uint8_t futureKeyFlag = pdu.getValue(pos, 1);
    m_report->add("future key flag", pdu.getValue(pos, 1));
    pos += 1;

    if (futureKeyFlag)
    {
        m_report->add("sck number", pdu.getValue(pos, 5));
        pos += 5;
        m_report->add("sck version number", pdu.getValue(pos, 16));
        pos += 16;
        m_report->add("sealed sck", pdu.getValue(pos, 120));
        pos += 120;
    }
    return pos;
}

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
        // 16.10.35a

        switch (val)
        {
        case 0b000:
            valueAsString = "Roaming location updating";
            break;
        case 0b001:
            valueAsString = "Temporary registration";
            break;
        case 0b010:
            valueAsString = "Periodic location updating";
            break;
        case 0b011:
            valueAsString = "ITSI attach";
            break;
        case 0b100:
            valueAsString = "Call restoration roaming location updating";
            break;
        case 0b101:
            valueAsString = "Migrating or call restoration migrating location updating";
            break;
        case 0b110:
            valueAsString = "Demand location updating";
            break;
        case 0b111:
            valueAsString = "Disabled MS updating";
            break;
        }
    }
    if (strEqualsU(key, "location update type"))
    {
        // 16.10.35

        switch (val)
        {
        case 0b000:
            valueAsString = "Roaming location updating";
            break;
        case 0b001:
            valueAsString = "Migrating location updating";
            break;
        case 0b010:
            valueAsString = "Periodic location updating";
            break;
        case 0b011:
            valueAsString = "ITSI attach";
            break;
        case 0b100:
            valueAsString = "Call restoration roaming location updating";
            break;
        case 0b101:
            valueAsString = "Call restoration migrating location updating";
            break;
        case 0b110:
            valueAsString = "Demand location updating";
            break;
        case 0b111:
            valueAsString = "Disabled MS updating";
            break;

        default:
            valueAsString = "unknown";
            break;
        }
    }
    if (strEqualsU(key, "energy saving mode"))
    {
        // 16.10.9 Table 16.38

        switch (val)
        {
        case 0b000:
            valueAsString = "Stay Alive";
            break;
        case 0b001:
            valueAsString = "Economy mode 1 (EG1)";
            break;
        case 0b010:
            valueAsString = "Economy mode 2 (EG2)";
            break;
        case 0b011:
            valueAsString = "Economy mode 3 (EG3)";
            break;
        case 0b100:
            valueAsString = "Economy mode 4 (EG4)";
            break;
        case 0b101:
            valueAsString = "Economy mode 5 (EG5)";
            break;
        case 0b110:
            valueAsString = "Economy mode 6 (EG6)";
            break;
        case 0b111:
            valueAsString = "Economy mode 7 (EG7)";
            break;
        }
    }
    if (strEqualsU(key, "LA timer"))
    {
        // 16.10.33 Table 16.63

        switch (val)
        {
        case 0b000:
            valueAsString = "10 min";
            break;
        case 0b001:
            valueAsString = "30 min";
            break;
        case 0b010:
            valueAsString = "1 hour";
            break;
        case 0b011:
            valueAsString = "2 hours";
            break;
        case 0b100:
            valueAsString = "4 hours";
            break;
        case 0b101:
            valueAsString = "8 hours";
            break;
        case 0b110:
            valueAsString = "24 hours";
            break;
        case 0b111:
            valueAsString = "no timing";
        break;
        }
    }
    if (strEqualsU(key, "reject cause"))
    {
        switch (val)
        {
        case 0b00001:
            valueAsString = "ITSI/ATSI unknown (system rejection)";
            break;
        case 0b00010:
            valueAsString = "Illegal MS (system rejection)";
            break;
        case 0b00011:
            valueAsString = "LA not allowed (LA rejection)";
            break;
        case 0b00100:
            valueAsString = "LA unknown (LA rejection)";
            break;
        case 0b00101:
            valueAsString = "Network failure (cell rejection)";
            break;
        case 0b00110:
            valueAsString = "Congestion (cell rejection)";
            break;
        case 0b00111:
            valueAsString = "Forward registration failure (cell rejection)";
            break;
        case 0b01000:
            valueAsString = "Service not subscribed (LA rejection)";
            break;
        case 0b01001:
            valueAsString = "Mandatory element error (system rejection)";
            break;
        case 0b01010:
            valueAsString = "Message consistency error (system rejection)";
            break;
        case 0b01011:
            valueAsString = "Roaming not supported (LA rejection)";
            break;
        case 0b01100:
            valueAsString = "Migration not supported (LA rejection)";
            break;
        case 0b01101:
            valueAsString = "No cipher KSG (cell rejection)";
            break;
        case 0b01110:
            valueAsString = "Identified cipher KSG not supported (cell rejection)";
            break;
        case 0b01111:
            valueAsString = "Requested cipher key type not available (cell rejection)";
            break;
        case 0b10000:
            valueAsString = "Identified cipher key not available (cell rejection)";
            break;
        case 0b10010:
            valueAsString = "Ciphering required (cell rejection)";
            break;
        case 0b10011:
            valueAsString = "Authentication failure (system rejection)";
            break;

        default:
            valueAsString = "reserved";
            break;
        }
    }
    if (strEqualsU(key, "type 3/4 element identifier"))
    {
        // 16.10.51 Table 16.89

        switch (val)
        {
        case 0b0000:
            valueAsString = "Reserved for future extension";
            break;
        case 0b0001:
            valueAsString = "Default group attachment lifetime";
            break;
        case 0b0010:
            valueAsString = "New registered area";
            break;
        case 0b0011:
            valueAsString = "Group identity location demand";
            break;
        case 0b0100:
            valueAsString = "Group report response";
            break;
        case 0b0101:
            valueAsString = "Group identity location accept";
            break;
        case 0b0110:
            valueAsString = "DM-MS address";
            break;
        case 0b0111:
            valueAsString = "Group identity downlink";
            break;
        case 0b1000:
            valueAsString = "Group identity uplink";
            break;
        case 0b1001:
            valueAsString = "Authentication uplink";
            break;
        case 0b1010:
            valueAsString = "Authentication downlink";
            break;
        case 0b1011:
            valueAsString = "Extended capabilities";
            break;
        case 0b1100:
            valueAsString = "Group Identity Security Related Information";
            break;
        case 0b1101:
            valueAsString = "Reserved for any future specified Type 3/4 element";
            break;
        case 0b1110:
            valueAsString = "Reserved for any future specified Type 3/4 element";
            break;
        case 0b1111:
            valueAsString = "Proprietary";
            break;
        }
    }
    return valueAsString;
}
