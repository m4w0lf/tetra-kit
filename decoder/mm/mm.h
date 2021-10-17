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
    };

};

#endif /* MM_H */
