#ifndef MLE_H
#define MLE_H
#include <sstream>
#include "../common/tetra.h"
#include "../common/layer.h"
#include "../common/log.h"
#include "../common/report.h"
#include "../common/utils.h"
#include "../cmce/cmce.h"
#include "../sndcp/sndcp.h"

namespace Tetra {

    class Mle : public Layer {
    public:
        Mle(Log * log, Report * report, Layer * cmce, Sndcp * sndcp);
        ~Mle();

        void service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress);

    private:
        Layer * m_cmce;
        Layer * m_sndcp;
        
        void serviceMleSubsystem(Pdu pdu, MacLogicalChannel macLogicalChannel);
        void processDNwrkBroadcast(Pdu pdu);
        void processDNwrkBroadcastExtension(Pdu pdu);

        uint32_t parseNeighbourCellInformation(Pdu data, uint32_t posStart, std::vector<std::tuple<std::string, uint64_t>> & infos);
    };

};

#endif /* MLE_H */
