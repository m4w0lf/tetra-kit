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
