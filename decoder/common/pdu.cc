#include "pdu.h"

using namespace Tetra;

/**
 * @brief Constructor of empty PDU
 *
 */

Pdu::Pdu()
{
    m_vec.clear();
}

/**
 * @brief Constructor from another PDU, starting at specified position with specified length
 *
 * NOTES:
 *   - if length is negative, null Pdu is created
 *   - if length is 0, input pdu is copied
 *   - if length is positive, input pdu is copied with at more length bytes (may be smaller depending on remainig characters)
 *
 */

Pdu::Pdu(const Pdu & pdu, const uint32_t startPos, const int32_t length)
{
    // clear internal vector
    m_vec.clear();

    // calculate actual length after start position
    int32_t actualLen = (int32_t)pdu.m_vec.size() - (int32_t)startPos;

    // check if we have remaining data after start position and requested length is positive or null (ie. whole pdu copy)
    if ((actualLen > 0) && (length >= 0))
    {
        if ((length > 0) && (actualLen > length))
        {
            //  we have more data available than requested, limit actualLen to requested length
            actualLen = length;
        }

        m_vec.assign(pdu.m_vec.begin() + (size_t)startPos, pdu.m_vec.begin() + (size_t)startPos + (size_t)actualLen);
    }
}


/**
 * @brief Constructor from a vector data
 *
 * We use the "copy" operator with b = const vector & a
 *
 */

Pdu::Pdu(const std::vector<uint8_t> & val)
{
    m_vec = val;
}

/**
 * @brief Destructor
 *
 */

Pdu::~Pdu()
{

}

/**
 * @brief
 *
 */

void Pdu::clear()
{
    m_vec.clear();
}

/**
 * @brief
 *
 */

uint64_t Pdu::getValue(const uint64_t startPos, const uint8_t fieldLen)
{
    uint64_t val = 0;

    for (uint64_t pos = 0; (pos < fieldLen) && (pos + startPos < m_vec.size()); pos++)
    {
        val += (m_vec[pos + startPos] << (fieldLen - pos - 1));
    }

    return val;
}

/**
 * @brief
 *
 */

void Pdu::append(const uint8_t val)
{
    m_vec.push_back(val);
}

/**
 * @brief
 *
 */

void Pdu::append(const std::vector<uint8_t> vec)
{
    m_vec.insert(m_vec.end(), vec.begin(), vec.end());
}

/**
 * @brief
 *
 */

void Pdu::append(const Pdu val)
{
    append(val.m_vec);
}

/**
 * @brief
 *
 */

std::string Pdu::toString(const int len)
{
    std::string res   = "";
    std::size_t count = m_vec.size();

    if ((len > 0) && (len < (int)count))                                        // if len is 0 then print entire vector
    {
        count = (std::size_t)len;
    }

    for (std::size_t idx = 0; idx < count; idx++)
    {
        res += (char)(m_vec[idx] + '0');
    }

    return res;
}

/**
 * @brief
 *
 */

std::string Pdu::toHex()
{
    std::string txt = "";
    char buf[32] = "";

    uint32_t pos = 0;
    for (std::size_t cnt = 0; cnt < m_vec.size() / 8; cnt++)
    {
        uint8_t val = getValue(pos, 8);
        pos += 8;

        if (cnt > 0)
        {
            sprintf(buf, " %02x", val);
        }
        else
        {
            sprintf(buf, "%02x", val);
        }

        txt += buf;
    }

    return txt;
}

/**
 * @brief
 *
 */

void Pdu::print(const int len)
{
    std::cout << toString(len) << std::endl;
}

/**
 * @brief
 *
 */

void Pdu::resize(const std::size_t len)
{
    if ((len > 0) && (len < m_vec.size()))
    {
        m_vec.resize(len);
    }
}

/**
 * @brief
 *
 */

std::size_t Pdu::size()
{
    return m_vec.size();
}

/**
 * @brief
 *
 */

uint8_t Pdu::at(const std::size_t pos)
{
    if (pos < m_vec.size())
    {
        return m_vec[pos];
    }
    else
    {
        return 0;
    }
}

/**
 * @brief
 *
 */

bool Pdu::isEmpty()
{
    return (m_vec.size() == 0);
}

/**
 * @brief Extract uint8_t vector from uint8_t vector
 *
 */

std::vector<uint8_t> Pdu::extractVec(const uint32_t startPos, const int32_t length)
{
    std::vector<uint8_t> ret;

    if (length > 0)                                                             // valid length requested
    {
        int32_t len = (int32_t)m_vec.size() - (int32_t)startPos;                // actual remaining bytes in vector after pos

        if (len > 0)                                                            // check if actual length is valid
        {
            if (len > length)                                                   // we have more bytes than requested
            {
                len = length;                                                   // so return only the requested ones
            }

            std::copy(m_vec.begin() + startPos, m_vec.begin() + startPos + (uint32_t)len, back_inserter(ret));
        }
    }

    return ret;
}

/**
 * @brief Decode 7-bit alphabet 29.5.4.3
 *
 */

std::string Pdu::textGsm7BitDecode(const int16_t len)
{
    // NOTE: _ is a special char when we want to escape the character value
    //                   0        10         20        30         40        50        60        70        80        90        100       110      120
    //                   0123456789012345678901234567890123 4567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
    const std::string GSM7 = "@_______________________________ !\"#{%&'()*+,-./0123456789:;<=>?_ABCDEFGHIJKLMNOPQRSTUVWXYZ______abcdefghijklmnopqrstuvwxyz_____";
    std::string res = "";

    for (int16_t idx = 0; idx < len / 7; idx++)
    {
        uint8_t chr = getValue((uint64_t)idx * 7, 7);

        char val = GSM7[chr];
        if (val == '{')
        {
            char buf[16] = "";
            sprintf(buf, "0x%02x", chr);
            res += buf;
        }
        else
        {
            res += (char)chr;
        }
    }

    return res;
}

/**
 * @brief Decode 8-bit alphabets TODO very rough function only, doesn't check alphabet input type
 *        Unknown symbols are replaced with '_' since we already have hex dump
 *
 */

std::string Pdu::textGeneric8BitDecode(const int16_t len)
{
    std::string res = "";

    for (int16_t idx = 0; idx < len / 8; idx++)
    {
        uint8_t chr = getValue((uint64_t)idx * 8, 8);
        if (isprint(chr))
        {
            res += (char)chr;
        }
        else
        {
            res += '_';
        }
    }

    return res;
}

/**
 * @brief NMEA location decode (NMEA 0183)
 *        ASCII-8 plain text with CR/LF endline
 *        with optional checksum separated by *
 *
 */

std::string Pdu::locationNmeaDecode(const int16_t len)
{
    std::string res = "";

    for (int16_t idx = 0; idx < len / 8; idx++)
    {
        uint8_t chr = getValue((uint64_t)idx * 8, 8);

        if ((chr != 10) && (chr != 13))                                         // skip CR/LF
        {
            res += (char)chr;
        }
    }

    return res;
}
