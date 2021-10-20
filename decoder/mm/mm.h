#ifndef MM_H
#define MM_H
#include <sstream>
#include "../common/tetra.h"
#include "../common/layer.h"
#include "../common/log.h"
#include "../common/report.h"
#include "../common/utils.h"

namespace Tetra {

    class Mm : public Layer {
    public:
        Mm(Log * log, Report * report);
        ~Mm();

        void service(Pdu pdu, const MacLogicalChannel macLogicalChannel, TetraTime tetraTime, MacAddress macAddress);

    private:
        void parseXXX(Pdu pdu);
        void parseDOtar(Pdu pdu);
        void parseDOtarCckProvide(Pdu pdu);
        void parseDOtarSckProvide(Pdu pdu);
        void parseDOtarSckReject(Pdu pdu);
        void parseDOtarGckProvide(Pdu pdu);
        void parseDOtarGckReject(Pdu pdu);
        void parseDOtarKeyAssociateDemand(Pdu pdu);
        void parseDOtarNewcell(Pdu pdu);
        void parseDOtarGskoProvide(Pdu pdu);
        void parseDOtarGskoReject(Pdu pdu);
        void parseDOtarKeyDeleteDemand(Pdu pdu);
        void parseDOtarKeyStatusDemand(Pdu pdu);
        void parseDOtarCmgGtsiProvide(Pdu pdu);
        void parseDOtarDmSckActivate(Pdu pdu);
        void parseDAuthentication(Pdu pdu);
        void parseDAuthenticationDemand(Pdu pdu);
        void parseDAuthenticationResponse(Pdu pdu);
        void parseDAuthenticationResult(Pdu pdu);
        void parseDAuthenticationReject(Pdu pdu);
        void parseDCkChangeDemand(Pdu pdu);
        void parseDDisable(Pdu pdu);
        void parseDEnable(Pdu pdu);
        void parseDLocationUpdateAccept(Pdu pdu);
        void parseType34Elements(Pdu pdu, uint64_t pos);
        void parseDLocationUpdateCommand(Pdu pdu);
        void parseDLocationUpdateReject(Pdu pdu);
        void parseDLocationUpdateProceeding(Pdu pdu);
        void parseDAttachDetachGroupIdentity(Pdu pdu);
        void parseDAttachDetachGroupIdentityAck(Pdu pdu);
        void parseDMmStatus(Pdu pdu);
        std::string valueToString(std::string key, uint32_t val);
    };

};

#endif /* MM_H */
