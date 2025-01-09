/*
 *  tetra-kit
 *  Copyright (C) 2020  LarryTh <dev@logami.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "cid.h"
#include "window.h"

/*
 * Simple Tetra recorder with ncurses ui
 *
 * Read informations and speech frames from decoder on UDP socket
 *
 * HISTORY:
 *   2016-09-11  LT  0.0  First release
 *   2020-05-08  LT  0.1  Updated for Json input
 *   2020-07-19  LT  0.2  Added internal TETRA codec and raw file output format
 *   2020-12-14  LT  0.3  Fixed zlib type mismatch for MacOS
 *   2021-05-05  LT  0.4  Raw speech frame output is now the default, removed -a option, added -x option
 *                        to get old behaviour (ie. don't process raw with internal speech codec).
 *
 * NOTES:
 *   - output log file is always generated, default name is 'log.txt'
 *   - output log files can be replayed with option -i
 *
 * MISCELLANEOUS:
 *    ssi              Short Suscriber Identity, identify all devices
 *    call_identifier  Call identifier, all communication are based on this
 *    usage_marker     (see 21.4.3.1 MAC-RESOURCE PDU address = SSI 24 bits + usage marker 6 bits (2^6=64)
 *                     possible values in usage marker 21.4.7 -> reserved 000000, 000001, 000010, 000011
 *
 * Filtering log for SDS: sed -n '/SDS/ p' log.txt > out.txt
 *
 */

/**
 * @brief Receive from UDP socket with time-out
 *
 */

int timed_recv(int fd_sock_rx, char * msg, std::size_t max_size, int max_wait_ms)
{
    struct sockaddr_in saddr_in;
    socklen_t socket_len = sizeof(saddr_in);

    fd_set fdset;
    FD_ZERO(&fdset);                                                            // clear fd set
    FD_SET(fd_sock_rx, &fdset);                                                 // add socket fd to set

    struct timeval timeout;
    timeout.tv_sec  =  max_wait_ms / 1000;
    timeout.tv_usec = (max_wait_ms % 1000) * 1000;

    int ret = select(fd_sock_rx + 1, &fdset, &fdset, &fdset, &timeout);         // monitor socket for max_wait_ms
    int val = -1;

    if (ret > 0)                                                                // socket has data, then receive it
    {
        val = recvfrom(fd_sock_rx, msg, max_size, 0, (struct sockaddr *)&saddr_in, &socket_len);
    }
    else                                                                        // invalid or no data
    {
        val = -1;
    }

    return val;
}

/** @brief Program working mode enumeration */

enum program_mode_t {
    STANDARD_MODE            = 0,
    READ_FROM_JSON_TEXT_FILE = 1
};

/** @brief interrupt flag */

static volatile int sigint_flag = 0;

/**
 * @brief handle SIGINT to clean up
 *
 */

static void sigint_handler(int val)
{
    sigint_flag = 1;
}

/**
 * @brief Program entry point
 *
 */

int main(int argc, char * argv[])
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, 0);

    int udp_port_rx = 42100;                                                    // UDP RX port (ie. where to receive Json text from decoder)

    const int FILENAME_LEN = 256;
    char opt_filename_in[FILENAME_LEN]  = "";                                   // input Json text filename
    char opt_filename_out[FILENAME_LEN] = "log.txt";                            // output Json text filename

    int program_mode     = STANDARD_MODE;
    int line_length      = 200;                                                 // default line length
    int max_bottom_lines = 20;                                                  // default bottom lines count
    int raw_format_flag  = 1;

    int option;
    while ((option = getopt(argc, argv, "xr:i:o:l:n:h")) != -1)
    {
        switch (option)
        {
        case 'x':
            raw_format_flag = 0;
            break;

        case 'r':
            udp_port_rx = atoi(optarg);
            break;

        case 'i':
            strncpy(opt_filename_in, optarg, FILENAME_LEN - 1);
            program_mode |= READ_FROM_JSON_TEXT_FILE;
            break;

        case 'o':
            strncpy(opt_filename_out, optarg, FILENAME_LEN - 1);
            break;

        case 'l':
            line_length = atoi(optarg);
            break;

        case 'n':
            max_bottom_lines = atoi(optarg);
            break;

        case 'h':
            printf("\nUsage: ./recorder [OPTIONS]\n\n"
                   "Options:\n"
                   "  -x don't process raw speech output with internal codec\n"
                   "  -r <UDP socket> receiving Json data from decoder [default port is 42100]\n"
                   "  -i <file> replay data from Json text file instead of UDP\n"
                   "  -o <file> to record Json data in different text file [default file name is 'log.txt'] (can be replayed with -i option)\n"
                   "  -l <ncurses line length> maximum characters printed on a report line\n"
                   "  -n <maximum lines in ssi window> ssi window will wrap when max. lines are printed\n"
                   "  -h print this help\n\n");
            exit(EXIT_FAILURE);
            break;

        case '?':
            printf("unkown option, run ./recorder -h to list available options\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    mkdir("out", S_IRWXU | S_IRGRP | S_IXGRP);                                  // create out/ directory
    mkdir("raw", S_IRWXU | S_IRGRP | S_IXGRP);                                  // create raw/ directory

    FILE * file_in = NULL;                                                      // for Json text read
    int fd_input = 0;                                                           // for UDP read

    if (program_mode & READ_FROM_JSON_TEXT_FILE)                                // read input text from file
    {
        file_in = fopen(opt_filename_in, "rt");

        if (file_in == NULL)
        {
            fprintf(stderr, "Couldn't open input Json text file");
            exit(EXIT_FAILURE);
        }
    }
    else                                                                        // read input bits from UDP socket
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(udp_port_rx);
        inet_aton("0.0.0.0", &addr.sin_addr);

        fd_input = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        bind(fd_input, (struct sockaddr *)&addr, sizeof(struct sockaddr));

        if (fd_input < 0)
        {
            fprintf(stderr, "Input socket 0x%04x on port %d\n", fd_input, udp_port_rx);
            fprintf(stderr, "Couldn't create input socket");
            exit(EXIT_FAILURE);
        }
    }

    // output log file
    FILE * file_out = fopen(opt_filename_out, "at");                            // default name is 'log.txt' unless modified by user with options

    if (file_out == 0)
    {
        fprintf(stderr, "Couldn't open output Json text file");
        exit(EXIT_FAILURE);
    }

    // initialize display and CID list
    scr_init(line_length, max_bottom_lines);
    cid_init(raw_format_flag);

    const int RX_BUFLEN = 65535;
    char rx_buf[RX_BUFLEN];

    if (program_mode & READ_FROM_JSON_TEXT_FILE)                                // read from Json text file file_in
    {
        while (!sigint_flag)
        {
            memset(rx_buf, 0, sizeof(rx_buf));
            while (fgets(rx_buf, sizeof(rx_buf), file_in))
            {
                std::string data(rx_buf);
                cid_parse_pdu(data, file_out);
            }
        }

        fclose(file_in);
    }
    else                                                                        // read from UDP socket fd_input
    {
        const int TIME_WAIT_MS = 250;                                            // udp port maximum waiting time [ms]

        while (!sigint_flag)
        {
            memset(rx_buf, 0, sizeof(rx_buf));
            int len = timed_recv(fd_input, rx_buf, RX_BUFLEN, TIME_WAIT_MS);

            if (len > 32)                                                        // skip small packets
            {
                std::string data(rx_buf);
                cid_parse_pdu(data, file_out);
            }
        }

        close(fd_input);
    }

    fclose(file_out);

    scr_clear();
    cid_clear();

    printf("Clean exit\n");

    return EXIT_SUCCESS;
}
