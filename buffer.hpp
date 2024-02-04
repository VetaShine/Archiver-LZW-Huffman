#ifndef BITSBUFFER_HPP
#define BITSBUFFER_HPP

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

class InputBuffer {
    public:
        InputBuffer(std::istream& input): file(input) {
            buffer = 0; 
            counter = 0;
        }

        uint32_t Get(uint32_t length) {
            uint32_t symbol;

            if (length > counter) {
                Read(length);
            }

            if (length > counter) {
                uint32_t storage = buffer;
                file.peek();
                buffer = 0;
                counter = 0;
                return storage;
            }

            counter -= length;
            symbol = (uint32_t)(buffer >> counter);

            if (counter != 0) {
                buffer = buffer << (64 - counter);
                buffer = buffer >> (64 - counter);
            } else {
                symbol = buffer;
                buffer = 0;
            }

            return symbol;
        }

        bool Empty() {
            if (counter == 0 && file.peek() == EOF) {
                return true;
            }
            return false;
        }

        void Cleaning() {
            buffer = 0;
            counter = 0;
        }

    private:
        std::istream& file;
        uint64_t buffer;
        uint32_t counter;

        void Read(uint32_t length) {
            uint8_t byte;

            while (length > counter) {
                file.read((char*)&byte, 1);
                counter += 8;
                buffer = (buffer << 8) | (uint64_t)(byte);
                file.peek();
            }
        }
};

class OutputBuffer {
    public:
        OutputBuffer(std::ostream& output) : file(output) {
            buffer = 0; 
            counter = 0;
            }

        void Add(uint8_t code) {
            counter += 8;
            buffer = (buffer << 8) | (uint64_t)code;
            Write();
        }

        void Add(uint16_t code) {
            counter += 16;
            buffer = (buffer << 16) | (uint64_t)code;
            Write();
        }

        void Add(uint32_t code, uint32_t length) {
            counter += length;
            buffer = (buffer << length) | (uint64_t)code;

            if (counter >= 24) {
                Write();
            }
        }

        void Add(std::vector<bool>& code) {
            for (int index = 0; index < code.size(); index++) {
                bool value = code[index];

                if (!value) {
                    buffer = (buffer << 1);
                } else {
                    buffer = (buffer << 1) | 1;
                }

                if (counter++ >= 32) {
                    Write();
                }
            }
        }

        void Add(bool code) {
            if (!code) {
                buffer = (buffer << 1);
            } else {
                buffer = (buffer << 1) | 1;
            }

            if (counter++ >= 24) {
                Write();
            }
        }

        void Cleaning() {
            Write();
            uint8_t byte = 0;

            if (counter == 0) {
                return;
            }

            byte = (uint8_t)(buffer << (8 - counter));
            file.write((char*)&byte, 1);
            buffer = 0;
            counter = 0;
        }

        virtual ~OutputBuffer() {}

    private:
        std::ostream& file;
        uint64_t buffer;
        uint32_t counter;

        void Write() {
            uint8_t byte;

            while (counter >= 8) {
                counter -= 8;
                byte = (uint8_t)(buffer >> (counter));

                if (counter != 0) {
                    buffer = buffer << (64 - counter);
                    buffer = buffer >> (64 - counter);
                } else {
                    buffer = 0;
                }
                
                file.write((char*)&byte, 1);
            }
        }
};

#endif
