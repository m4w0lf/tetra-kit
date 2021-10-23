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
                                                                                // Not used by downlink PDUs
            break;
        case 0b0100:                                                            // Group report response
            elementType = 3;                                                    // According to Table 16.1
            break;
        case 0b0101:                                                            // Group identity location accept
            elementType = 3;                                                    // According to Table 16.11
            break;
        case 0b0110:                                                            // DM-MS address
                                                                                // Used only in DMO
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
                                                                                // not used by downlink PDUs
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
            case 0b1111:
                pos = parseProprietary(pdu, pos);
                break;
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

    bool ckProvisionFlag = pdu.getValue(pos, 1);
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
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_cck_information", pdu.toString().c_str());

    m_report->add("cck identifier", pdu.getValue(pos, 16));
    pos += 16;
    m_report->add("key type flag", pdu.getValue(pos, 1));
    pos += 1;
    m_report->add("sealed cck", pdu.getValue(pos, 120));
    pos += 120;
    m_report->add("cck location area information", pdu.getValue(pos, 2));
    pos += 2;

    bool futureKeyFlag = pdu.getValue(pos, 1);
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

    bool securityClass = pdu.getValue(pos, 1);
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
        // reserved
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

    bool sckProvisionFlag = pdu.getValue(pos, 1);
    pos += 1;

    if (sckProvisionFlag)
    {
        pos = parseSckInformation(pdu, pos);
    }

    bool cckProvisionFlag = pdu.getValue(pos, 1);
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
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_gck_rejected", pdu.toString().c_str());

    m_report->add("otar reject reason", pdu.getValue(pos, 3));
    pos += 3;

    uint8_t groupAssociation = pdu.getValue(pos, 1);
    m_report->add("group association", pdu.getValue(pos, 1));
    pos += 1;

    if (groupAssociation)                                                       // Associated with specific GSSI
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

    bool attachDetachType = pdu.getValue(pos, 1);
    m_report->add("group identity attach/detach type identifier", pdu.getValue(pos, 1));
    pos += 1;

    if (attachDetachType)                                                       // detachment
    {
        m_report->add("group identity detachment downlink", pdu.getValue(pos, 2));
        pos += 2;
    }
    else                                                                        // attachment
    {
        m_report->add("group identity attachment lifetime", pdu.getValue(pos, 2));
        pos += 2;
        m_report->add("class of usage", pdu.getValue(pos, 3));
        pos += 3;
    }

    uint32_t groupIdentityAddressType = pdu.getValue(pos, 2);
    m_report->add("group identity address type", pdu.getValue(pos, 2));
    pos += 2;

    if (groupIdentityAddressType == 0)                                          // GSSI
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
    }
    if (groupIdentityAddressType == 1)                                          // GTSI
    {
        m_report->add("gssi", pdu.getValue(pos, 24));
        pos += 24;
        pos = parseAddressExtension(pdu, pos);
    }
    if (groupIdentityAddressType == 2)                                          // (V)GSSI
    {
        m_report->add("(v)gssi", pdu.getValue(pos, 24));
        pos += 24;
    }
    if (groupIdentityAddressType == 3)                                          // GTSI+(V)GSSI
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

    // reserved
    pos += 1;

    bool oBit = pdu.getValue(pos, 1);
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

    bool gckAssociation = pdu.getValue(pos, 1);
    pos += 1;

    if (gckAssociation)                                         // GCK association information provided
    {
        m_report->add("gck select number", pdu.getValue(pos, 17));
        pos += 17;
    }

    bool sckAssociation = pdu.getValue(pos, 1);
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

    bool oBit = pdu.getValue(pos, 1);
    pos += 1;

    if (oBit)
    {
        bool pBit = pdu.getValue(pos, 1);
        pos += 1;

        if (pBit)
        {
            m_report->add("LACC", pdu.getValue(pos, 10));
            pos += 10;
            m_report->add("LANC", pdu.getValue(pos, 14));
            pos += 14;
        }
    }

    return pos;
}


/**
 * @brief MM New registered area - 16.10.40
 *
 */

uint64_t Mm::parseProprietary(Pdu pdu, uint64_t pos)
{
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_new_registered_area", pdu.toString().c_str());

    uint32_t proprietaryElementOwner = pdu.getValue(pos, 8);
    m_report->add("proprietary element owner", pdu.getValue(pos, 8));
    pos += 8;

    if (proprietaryElementOwner == 0)
    {
        // proprietary element, cannot be parsed
        /*
        m_report->add("proprietary element owner extension", pdu.getValue(pos, 1));
        pos += 1;
        */
    }

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
    m_log->print(LogLevel::HIGH, "DEBUG ::%-44s - pdu = %s\n", "mm_parse_sck_information", pdu.toString().c_str());

    bool sessionKey = pdu.getValue(pos, 1);
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

    bool futureKeyFlag = pdu.getValue(pos, 1);
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

/**
 * @brief Find value in map
 *
 */

std::string Mm::getMapValue(std::map<uint32_t, std::string> informationElement, uint32_t val)
{
    // get the default base value
    std::string res = Layer::getMapValue(informationElement, val);

    // map value was not found, so return reserved keyword instead
    if (strEqualsU(res, "not found"))
    {
        res = "reserved";
    }

    return res;
}

/**
 * @brief Convert MM elements value to string
 *
 */

std::string Mm::valueToString(std::string key, uint32_t val)
{
    if (strEqualsU(key, "energy saving mode"))
    {
        // 16.10.9 Table 16.38

        std::map<uint32_t, std::string> energySavingMode
        {
            {0b000, "Stay Alive"},
            {0b001, "Economy mode 1 (EG1)"},
            {0b010, "Economy mode 2 (EG2)"},
            {0b011, "Economy mode 3 (EG3)"},
            {0b100, "Economy mode 4 (EG4)"},
            {0b101, "Economy mode 5 (EG5)"},
            {0b110, "Economy mode 6 (EG6)"},
            {0b111, "Economy mode 7 (EG7)"}
        };

        return getMapValue(energySavingMode, val);
    }
    if (strEqualsU(key, "LA timer"))
    {
        // 16.10.33 Table 16.63

        std::map<uint32_t, std::string> laTimer
        {
            {0b000, "10 min"},
            {0b001, "30 min"},
            {0b010, "1 hour"},
            {0b011, "2 hours"},
            {0b100, "4 hours"},
            {0b101, "8 hours"},
            {0b110, "24 hours"},
            {0b111, "no timing"}
        };

        return getMapValue(laTimer, val);
    }
    if (strEqualsU(key, "location update accept type"))
    {
        // 16.10.35a

        std::map<uint32_t, std::string> locationUpdateAcceptType
        {
            {0b000, "Roaming location updating"},
            {0b001, "Temporary registration"},
            {0b010, "Periodic location updating"},
            {0b011, "ITSI attach"},
            {0b100, "Call restoration roaming location updating"},
            {0b101, "Migrating or call restoration migrating location updating"},
            {0b110, "Demand location updating"},
            {0b111, "Disabled MS updating"}
        };

        return getMapValue(locationUpdateAcceptType, val);
    }
    if (strEqualsU(key, "location update type"))
    {
        // 16.10.35

        std::map<uint32_t, std::string> locationUpdateType
        {
            {0b000, "Roaming location updating"},
            {0b001, "Migrating location updating"},
            {0b010, "Periodic location updating"},
            {0b011, "ITSI attach"},
            {0b100, "Call restoration roaming location updating"},
            {0b101, "Call restoration migrating location updating"},
            {0b110, "Demand location updating"},
            {0b111, "Disabled MS updating"}
        };

        return getMapValue(locationUpdateType, val);
    }
    if (strEqualsU(key, "pdu type"))
    {
        // 16.10.39

        std::map<uint32_t, std::string> pduType
        {
            {0b0000, "D-OTAR"},
            {0b0001, "D-AUTHENTICATION"},
            {0b0010, "D-CK CHANGE DEMAND"},
            {0b0011, "D-DISABLE"},
            {0b0100, "D-ENABLE"},
            {0b0101, "D-LOCATION UPDATE ACCEPT"},
            {0b0110, "D-LOCATION UPDATE COMMAND"},
            {0b0111, "D-LOCATION UPDATE REJECT"},
            {0b1001, "D-LOCATION UPDATE PROCEEDING"},
            {0b1010, "D-ATTACH/DETACH GROUP IDENTITY"},
            {0b1011, "D-ATTACH/DETACH GROUP IDENTITY ACK"},
            {0b1100, "D-MM STATUS"},
            {0b1111, "MM PDU/FUNCTION NOT SUPPORTED"}
        };

        return getMapValue(pduType, val);
    }
    if (strEqualsU(key, "reject cause"))
    {
        std::map<uint32_t, std::string> rejectCause
        {
            {0b00001, "ITSI/ATSI unknown (system rejection)"},
            {0b00010, "Illegal MS (system rejection)"},
            {0b00011, "LA not allowed (LA rejection)"},
            {0b00100, "LA unknown (LA rejection)"},
            {0b00101, "Network failure (cell rejection)"},
            {0b00110, "Congestion (cell rejection)"},
            {0b00111, "Forward registration failure (cell rejection)"},
            {0b01000, "Service not subscribed (LA rejection)"},
            {0b01001, "Mandatory element error (system rejection)"},
            {0b01010, "Message consistency error (system rejection)"},
            {0b01011, "Roaming not supported (LA rejection)"},
            {0b01100, "Migration not supported (LA rejection)"},
            {0b01101, "No cipher KSG (cell rejection)"},
            {0b01110, "Identified cipher KSG not supported (cell rejection)"},
            {0b01111, "Requested cipher key type not available (cell rejection)"},
            {0b10000, "Identified cipher key not available (cell rejection)"},
            {0b10010, "Ciphering required (cell rejection)"},
            {0b10011, "Authentication failure"}
        };

        return getMapValue(rejectCause, val);
    }
    if (strEqualsU(key, "type 3/4 element identifier"))
    {
        // 16.10.51 Table 16.89

        std::map<uint32_t, std::string> type34ElementIdentifier
        {
            {0b0000, "Reserved for future extension"},
            {0b0001, "Default group attachment lifetime"},
            {0b0010, "New registered area"},
            {0b0011, "Group identity location demand"},
            {0b0100, "Group report response"},
            {0b0101, "Group identity location accept"},
            {0b0110, "DM-MS address"},
            {0b0111, "Group identity downlink"},
            {0b1000, "Group identity uplink"},
            {0b1001, "Authentication uplink"},
            {0b1010, "Authentication downlink"},
            {0b1011, "Extended capabilities"},
            {0b1100, "Group Identity Security Related Information"},
            {0b1101, "Reserved for any future specified Type 3/4 element"},
            {0b1110, "Reserved for any future specified Type 3/4 element"},
            {0b1111, "Proprietary"}
        };

        return getMapValue(type34ElementIdentifier, val);
    }
    return "unknown";
}
