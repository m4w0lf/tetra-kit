#ifndef LAYER_H
#define LAYER_H
#include "tetra.h"
#include "log.h"
#include "report.h"
#include "pdu.h"

namespace Tetra {

    /**
     * @brief Base class for all layers MAC, LLC, MLE, UPLANE, MLE, CMCE
     *        and sub-layers SDS, LIP, SNDCP.
     *
     *        Upper service entry point is service()
     *
     */

    class Layer {
    public:
        Layer(Log * log, Report * report);
        virtual ~Layer();

        virtual void service(const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress);
        virtual void service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress);
        virtual void service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress, MacState macState, uint8_t encryptionMode);

    protected:
        Log    * m_log;                                                         ///< Screen logger
        Report * m_report;                                                      ///< UDP JSON report

        MacLogicalChannel m_macLogicalChannel;                                  ///< Current MAC logical channel
        TetraTime m_tetraTime;                                                  ///< Current service tetra time
        MacAddress m_macAddress;                                                ///< Current service MAC address
    };

};


#endif /* LAYER_H */
