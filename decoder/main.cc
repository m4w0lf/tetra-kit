#include <cstdio>
#include "decoder.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include <string>
#include <iostream>

#include <zmq.hpp>

/** @brief Program working mode enumeration */

enum ProgramMode {
    STANDARD_MODE         = 0,
    RX_PACKED             = 4,
};

/** @brief Interrupt flag */

static volatile int gSigintFlag = 0;

/**
 * @brief Handle SIGINT to clean up
 *
 */

static void sigint_handler(int val)
{
    gSigintFlag = 1;
}

/**
 * @brief Decoder program entry point
 *
 * Reads demodulated values from UDP port 42000 coming from physical demodulator
 * Writes decoded frames to UDP port 42100 to tetra interpreter
 *
 * Filtering log for SDS: sed -n '/SDS/ p' log.txt > out.txt
 *
 */

int main(int argc, char * argv[])
{
    // connect interrupt Ctrl-C handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, 0);

    int udpPortRx = 42000;                                                      // UDP RX port (ie. where to receive bits from PHY layer)

    int programMode = STANDARD_MODE;
    int debugLevel = 1;
    bool bRemoveFillBits = true;
    bool bEnableWiresharkOutput = false;

    char queueUrl[255] = "tcp://localhost:42100";     // initialize the zmq context with a single IO thread

    int option;
    while ((option = getopt(argc, argv, "hPwr:a:d:f")) != -1)
    {
        switch (option)
        {
        case 'r':
            udpPortRx = atoi(optarg);
            break;

        case 'P':
            programMode |= RX_PACKED;
            break;

        case 'd':
            debugLevel = atoi(optarg);
            break;

        case 'f':
            bRemoveFillBits = false;
            break;

        case 'w':
            bEnableWiresharkOutput = true;
            break;

        case 'a':
            strncpy(queueUrl, optarg, 255);

            break;

        case 'h':
            printf("\nUsage: ./decoder [OPTIONS]\n\n"
                   "Options:\n"
                   "  -r <UDP socket> receiving from phy [default port is 42000]\n"
                   "  -a ZMQ url for output Json data [default is tcp://localhost:42100]\n"
                   "  -d <level> print debug information\n"
                   "  -f keep fill bits\n"
                   "  -w enable wireshark output [EXPERIMENTAL]\n"
                   "  -P pack rx data (1 byte = 8 bits)\n"
                   "  -h print this help\n\n");
            exit(EXIT_FAILURE);
            break;

        case '?':
            printf("unkown option, run ./decoder -h to list available options\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    
    printf("Destination: %s\n", queueUrl);

    zmq::context_t zmqContext{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t zmqSocket{zmqContext, zmq::socket_type::push};
    zmqSocket.connect(queueUrl);

    // create decoder
    Tetra::LogLevel logLevel;
    switch (debugLevel)
    {
    case 0:
        logLevel = Tetra::LogLevel::NONE;
        break;
    case 1:
        logLevel = Tetra::LogLevel::LOW;
        break;
    case 2:
        logLevel = Tetra::LogLevel::MEDIUM;
        break;
    case 3:
        logLevel = Tetra::LogLevel::HIGH;
        break;
    case 4:
        logLevel = Tetra::LogLevel::VERYHIGH;
        break;
    default:
        logLevel = Tetra::LogLevel::LOW;

    }

    // input source
    int fdInput = 0;

    // read input bits from UDP socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(udpPortRx);
    inet_aton("0.0.0.0", &addr.sin_addr);

    fdInput = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(fdInput, (struct sockaddr *)&addr, sizeof(struct sockaddr));

    printf("Input socket 0x%04x on port %d\n", fdInput, udpPortRx);

    if (fdInput < 0)
    {
        fprintf(stderr, "Couldn't create input socket");
        exit(EXIT_FAILURE);
    }

    // create decoder
    Tetra::TetraDecoder * decoder = new Tetra::TetraDecoder(&zmqSocket, bRemoveFillBits, logLevel, bEnableWiresharkOutput);

    // receive buffer
    const int RXBUF_LEN = 1024;
    uint8_t rxBuf[RXBUF_LEN];

    while (!gSigintFlag)
    {
        int bytesRead = read(fdInput, rxBuf, sizeof(rxBuf));

        if (errno == EINTR)
        {
            // print is required for ^C to be handled
            fprintf(stderr, "EINTR\n");
            break;
        }
        else if (bytesRead < 0)
        {
            fprintf(stderr, "Read error\n");
            break;
        }
        else if (bytesRead == 0)
        {
            break;
        }

        // bytes must be pushed one at a time into decoder
        for (int cnt = 0; cnt < bytesRead; cnt++)
        {
        	if (programMode & RX_PACKED)
        	{
        		for (uint8_t idx = 0; idx <= 7; idx++)
        	    {
        			decoder->rxSymbol((rxBuf[cnt] >> idx) & 0x01);
        		}
        	}
        	else
        	{
        		decoder->rxSymbol(rxBuf[cnt]);
        	}
        }
    }

    // file or socket must be closed
    close(fdInput);
    
    zmqSocket.close();
    zmqContext.close();

    delete decoder;

    printf("Clean exit\n");

    return EXIT_SUCCESS;
}
