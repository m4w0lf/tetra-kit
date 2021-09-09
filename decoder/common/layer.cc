#include "layer.h"

using namespace Tetra;

/**
 * @brief Constructor
 *
 */

Layer::Layer(Log * log, Report * report)
{
    m_log = log;
    m_report = report;
}

/**
 * @brief Destructor
 *
 */

Layer::~Layer()
{

}

/**
 * @brief Service upper layer
 *
 */

void Layer::service(const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress)
{
    m_macLogicalChannel = macLogicalChannel;
    m_tetraTime = tetraTime;
    m_macAddress = macAddress;
}

/**
 * @brief Service upper layer (all layers but UPlane)
 *
 */

void Layer::service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress)
{
    service(macLogicalChannel, tetraTime, macAddress);
}

/**
 * @brief Service upper layer (for UPlane with MAC state and encryption mode)
 *
 */

void Layer::service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress, MacState macState, uint8_t encryptionMode)
{
    service(macLogicalChannel, tetraTime, macAddress);
}
