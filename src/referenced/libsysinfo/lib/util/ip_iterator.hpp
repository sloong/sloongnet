/**
 * @author: Daniel Fuchs
 * @contact: fuxeysolutions@gmail.com
 *
 * distributed under the MIT License (MIT).
 * Copyright (c) Daniel Fuchs
 *
 */
#pragma once

#include <algorithm>


class ipv4_Range {
public:
    class ipv4_Address {
    public:

        ipv4_Address() {

        }

        ipv4_Address(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4) :
                ipv4_part1(p1), ipv4_part2(p2), ipv4_part3(p3), ipv4_part4(p4) {
            convertU8toU32();
        }

        std::string toString() const {
            return std::to_string(static_cast<int>(ipv4_part1)) + "." +
                   std::to_string(static_cast<int>(ipv4_part2)) + "." +
                   std::to_string(static_cast<int>(ipv4_part3)) + "." +
                   std::to_string(static_cast<int>(ipv4_part4));
        }

        bool operator==(const ipv4_Address &rhs) const {
            return ipv4 == rhs.ipv4;
        }

        bool operator>(const ipv4_Address &rhs) const {
            return ipv4 > rhs.ipv4;
        }

        bool operator<(const ipv4_Address &rhs) const {
            return ipv4 < rhs.ipv4;
        }

        ipv4_Address operator=(ipv4_Address &adr) {
            ipv4_Address newAdr(adr.ipv4_part1,
                                adr.ipv4_part2,
                                adr.ipv4_part3,
                                adr.ipv4_part4
            );
            return newAdr;
        }

        ipv4_Address operator++(int) {
            ipv4_Address i = *this;
            ipv4 += 1;
            convertU32toU8();
            return i;
        }

    private:
        uint8_t ipv4_part1;
        uint8_t ipv4_part2;
        uint8_t ipv4_part3;
        uint8_t ipv4_part4;
        uint32_t ipv4;

        void convertU32toU8() {
            ipv4_part1 = static_cast<uint8_t>(ipv4 >> 24);
            ipv4_part2 = static_cast<uint8_t>(ipv4 >> 16);
            ipv4_part3 = static_cast<uint8_t>(ipv4 >> 8);
            ipv4_part4 = static_cast<uint8_t>(ipv4);

        }

        void convertU8toU32() {
            ipv4 = (static_cast<uint32_t>(ipv4_part1) << 24) +
                   (static_cast<uint32_t>(ipv4_part2) << 16) +
                   (static_cast<uint32_t>(ipv4_part3) << 8) +
                   static_cast<uint32_t>(ipv4_part4);
        }

    };


    class iterator : public std::iterator<
            std::input_iterator_tag,
            ipv4_Address,
            ipv4_Address,
            const ipv4_Address *,
            ipv4_Address> {

        ipv4_Address start;
    public:
        explicit iterator(ipv4_Address startAdr) : start(startAdr) {}

        iterator &operator++() {
            start++;
            return *this;
        }

        bool operator!=(iterator other) const {
            return !(start == other.start);
        }

        bool operator==(iterator other) const {
            return start == other.start;
        }

        reference operator*() const { return start; }
    };


    ipv4_Range(ipv4_Address start, ipv4_Address end) : startAddress(start), endAddress(end) {
        validate();
    }

    iterator begin() {
        return iterator(startAddress);
    }

    iterator end() {
        return iterator(endAddress++);
    }

private:
    void validate() {
        if (startAddress == endAddress) {
            throw std::invalid_argument("start and end are equal");
        }
        if (startAddress > endAddress) {
            // ok
        } else {
            // switch
            ipv4_Address temp = startAddress;
            startAddress = endAddress;
            endAddress = temp;
        }
    }


    ipv4_Address startAddress;
    ipv4_Address endAddress;


};