#include <cstdio>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <cmath>
#include <sstream>
#include <experimental/filesystem>
#include "lzw.hpp"
#include "huffman.hpp"

int FileProccessing(std::string filename, uint32_t level, bool decompress, bool output_preservation, bool preservation, bool stdcin) {
    std::string result, name = filename + ".tmp";
    std::ifstream input, input_file;
    std::ofstream output, output_file;
    uint32_t pointer = 0;
    double original_size, final_size, coefficient;

    if (decompress && !stdcin) {
        output_file.open(name, std::ofstream::out | std::ofstream::binary);

        if (!(output_file.is_open())) {
            std::cerr << "Error while opening file " << name << '\n';
            output_file.close();
            return 1;
        }

        if (filename.substr(filename.length() - 2) != ".z") {
            std::cerr << "The file name must end with .z" << '\n';
            return 1;
        }

        input.open(filename, std::ifstream::in | std::ifstream::binary);

        if (!(input.is_open())) {
            std::cerr << "Error while opening file " << filename << std::endl;
            input.close();
            return 1;
        }

        uint8_t byte = 0xee;
        uint8_t byte_read;
        input.clear();
        input.read((char *)&byte_read, 1);

        if (byte_read == byte) {
            Decode(input, output_file);
        } else {
            pointer = 1;
        }

        input.close();
        output_file.close();

        if (pointer) {
            return pointer;
        }

        input_file.open(name, std::ifstream::in | std::ifstream::binary);

        if (!(input_file.is_open())) {
            std::cerr << "Error while opening file " << name << std::endl;
            input_file.close();
            return 1;
        }

        if (output_preservation) {
            std::ostream &standart_output = std::cout;
            Decompress(input_file, standart_output);
        } else {
            output.open(filename.substr(0, filename.length() - 2), std::ofstream::out | std::ofstream::binary);

            if (!(output.is_open())) {
                std::cerr << "Error while opening file " << filename.substr(0, filename.length() - 2) << std::endl;
                output.close();
                return 1;
            }

            pointer = Decompress(input_file, output);
            output.close();
        }

        input_file.close();
    } else {
        output_file.open(name, std::ofstream::out | std::ofstream::binary);

        if (!(output_file.is_open())) {
                std::cerr << "Error while opening file " << name << '\n';
                output_file.close();
                return 1;
            }

        if (!stdcin) {
            input.open(filename, std::ifstream::in | std::ifstream::binary);

            if (!(input.is_open())) {
                std::cerr << "Error while opening file " << filename << std::endl;
                input.close();
                return 1;
            }

            original_size = input.tellg();
            Compress(input, output_file, level);
            input.close();
        } else {
            std::istream &standart_input = std::cin;
            Compress(standart_input, output_file, level);
        }

        output_file.close();
        input_file.open(name, std::ifstream::in | std::ifstream::binary);

        if (!(input_file.is_open())) {
            std::cerr << "Error while opening file " << name << std::endl;
            input_file.close();
            return 1;
        }

        if (output_preservation || stdcin) {
            std::ostream &standart_output = std::cout;
            uint8_t byte = 0xee;
            standart_output.write((char *)&byte, 1);
            Encode(input_file, standart_output);
        } else {
            output.open(filename + ".z", std::ofstream::out | std::ofstream::binary);

            if (!(output.is_open())) {
                std::cerr << "Error while opening file " << filename + ".z" << std::endl;
                output.close();
                return 1;
            }

            uint8_t byte = 0xee;
            output.write((char *)&byte, 1);
            Encode(input_file, output);
            output.write((char *)&byte, 1);
            uint64_t size = std::experimental::filesystem::file_size(filename);
            output.write((char *)&size, 8);
            output.close();
        }

        input_file.close();
    }

    if (!output_preservation && !preservation && !stdcin) {
        std::remove(filename.c_str());
    }

    if (decompress) {
        return pointer;
    } else {
        return 0;
    }
}

void ProcessDirectory(const std::string& directory, uint32_t level, bool decompress, bool recursive, bool output_preservation, bool preservation, bool stdcin) {
    DIR* storage;
    struct dirent* entry;

    if ((storage = opendir(directory.c_str())) != nullptr) {
        while ((entry = readdir(storage)) != nullptr) {
            std::string filename = entry->d_name;
            std::string full_path = directory + "/" + filename;

            if (filename == "." || filename == ".." || filename.rfind(".", 0) == 0) {
                continue;
            }

            struct stat structure;
            if (stat(full_path.c_str(), &structure) != -1 && S_ISDIR(structure.st_mode)) {
                if (recursive) {
                ProcessDirectory(full_path, level, decompress, recursive, output_preservation, preservation, stdcin);
                }
            } else {
                if (decompress) {
                    FileProccessing(full_path, level, decompress, output_preservation, preservation, stdcin);
                } else {
                    FileProccessing(full_path, level, decompress, output_preservation, preservation, stdcin);
                }
            }
        }

        closedir(storage);
    } else {
        std::cerr << "Error opening directory: " << directory << '\n';
    }
}

int main(int argc, char* argv[]) {
    const char* short_options = "cdklr19t";
    const struct option long_options[] = {
        {"output_preservation", no_argument, nullptr, 'c'},
        {"decompress", no_argument, nullptr, 'd'},
        {"preservation", no_argument, nullptr, 'k'},
        {"information", no_argument, nullptr, 'l'},
        {"recursive", no_argument, nullptr, 'r'},
        {"level1", no_argument, nullptr, '1'},
        {"level9", no_argument, nullptr, '9'},
        {"test", no_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };

    bool output_preservation = false;
    bool decompress = false;
    bool preservation = false;
    bool information = false;
    bool recursive = false;
    bool level1 = false;
    bool level9 = false;
    bool test = false;
    bool stdcin = false;
    std::string filename;
    int option;

    while ((option = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
        switch (option) {
            case 'c':
                output_preservation = true;
                break;
            case 'd':
                decompress = true;
                break;
            case 'k':
                preservation = true;
                break;
            case 'l':
                information = true;
                break;
            case 'r':
                recursive = true;
                break;
            case '1':
                level1 = true;
                break;
            case '9':
                level9 = true;
                break;
            case 't':
                test = true;
                break;
            default:
                std::cerr << "Unknown option: " << optarg << '\n';
                return 1;
        }
    }

    if (optind < argc) {
        filename = argv[optind];
        
        if (filename == "-") {
            stdcin = true;
        }
    } else {
        std::cerr << "No filename provided" << '\n';
        return 1;
    }

    if (information && !output_preservation && !decompress && !preservation && !recursive && !level1 && !level9 && !test && !stdcin) {
        if (filename.substr(filename.length() - 2) == ".z") {
            std::ifstream file;
            double coefficient;
            uint64_t original_size, final_size = std::experimental::filesystem::file_size(filename);
            file.open(filename, std::ifstream::in | std::ifstream::binary);

            if (!(file.is_open())) {
                std::cerr << "Error while opening file " << filename << std::endl;
                file.close();
                return 1;
            }

            file.seekg(-8, file.end);
            file.read((char *)&original_size, 8);
            file.close();
            std::cout << "Options:\n";
            std::cout << "Uncompressed file size: " << original_size << '\n';
            std::cout << "Compressed file size: " << final_size << '\n';

            if (original_size > final_size) {
                coefficient = (round((((double)original_size - (double)final_size) / (double)original_size) * 1000.0)) / 10.0; 
            } else {
                coefficient = 0;
            }

            std::cout << "Ratio: " << coefficient << "%\n";
            std::cout << "Uncompressed file name: " << filename.substr(0, filename.length() - 2) << '\n';
            return 0;
        } else {
            std::cerr << "The file name must end with .z" << '\n';
            return 1;
        }
    } 

    if (test && !output_preservation && !decompress && !preservation && !recursive && !level1 && !level9 && !information && !stdcin) {
        if (filename.substr(filename.length() - 2) == ".z") {
            std::ifstream file;
            uint8_t byte;
            bool flag = false;
            file.open(filename, std::ifstream::in | std::ifstream::binary);

            if (!(file.is_open())) {
                std::cerr << "Error while opening file " << filename << std::endl;
                file.close();
                return 1;
            }

            file.read((char *)&byte, 1);

            if (byte != 0xee) {
                flag = true;
            } else {
                file.seekg(-9, file.end);
                file.read((char *)&byte, 1);

                if (byte != 0xee) {
                    flag = true;
                }
            }

            file.close();

            if (flag) {
                std::cout << "The file is incorrect" << '\n';
            } else {
                std::cout << "The file is correct" << '\n';
            }

            return 0;
        } else {
            std::cerr << "The file name must end with .z" << '\n';
            return 1;
        }
    } 

    uint32_t level = 14;

    if (level1) {
        level = 12;
    } else if (level9) {
        level = 16;
    }

    if (recursive) {
        ProcessDirectory(filename, level, decompress, recursive, output_preservation, preservation, stdcin);
    } else {
        FileProccessing(filename, level, decompress, output_preservation, preservation, stdcin);
    }

    return 0;
}
