#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

void collectFileSizes(const fs::path& folderPath, vector<uintmax_t>& fileSizes) {
    try {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            try {
                if (fs::is_regular_file(entry.path())) {
                    fileSizes.push_back(fs::file_size(entry.path()));
                }
                else if (fs::is_directory(entry.path())) {
                    collectFileSizes(entry.path(), fileSizes);
                }
            }
            catch (...) {
                // skip files or folders if access fails
            }
        }
    }
    catch (...) {
        // skip this folder if it cannot be opened
    }
}

map<uintmax_t, int> makeHistogram(const vector<uintmax_t>& fileSizes, uintmax_t binWidth) {
    map<uintmax_t, int> histogram;

    if (fileSizes.empty()) {
        return histogram;
    }

    uintmax_t minSize = fileSizes[0];
    uintmax_t maxSize = fileSizes[0];

    for (uintmax_t size : fileSizes) {
        if (size < minSize) {
            minSize = size;
        }
        if (size > maxSize) {
            maxSize = size;
        }
    }

    uintmax_t firstBin = (minSize / binWidth) * binWidth;
    uintmax_t lastBin = (maxSize / binWidth) * binWidth;

    for (uintmax_t bin = firstBin; bin <= lastBin; bin += binWidth) {
        histogram[bin] = 0;
    }

    for (uintmax_t size : fileSizes) {
        uintmax_t binStart = (size / binWidth) * binWidth;
        histogram[binStart]++;
    }

    return histogram;
}

void printHistogram(const map<uintmax_t, int>& histogram, uintmax_t binWidth) {
    cout << "\nfile size histogram:\n";

    for (const auto& bin : histogram) {
        uintmax_t start = bin.first;
        uintmax_t end = start + binWidth - 1;

        cout << start << " - " << end << " bytes : " << bin.second << "\n";
    }
}

void saveHistogramToCsv(const map<uintmax_t, int>& histogram, uintmax_t binWidth) {
    ofstream outFile("results.csv");

    if (!outFile.is_open()) {
        cout << "could not create csv file\n";
        return;
    }

    outFile << "bin_start,bin_end,file_count\n";

    for (const auto& bin : histogram) {
        uintmax_t start = bin.first;
        uintmax_t end = start + binWidth - 1;

        outFile << start << "," << end << "," << bin.second << "\n";
    }

    outFile.close();
}

int main() {
    string startDirectory;
    uintmax_t binWidth;

    cout << "enter start directory: ";
    getline(cin, startDirectory);

    cout << "enter bin width: ";
    cin >> binWidth;

    if (binWidth == 0) {
        cout << "bin width must be greater than 0\n";
        return 1;
    }

    vector<uintmax_t> fileSizes;

    collectFileSizes(startDirectory, fileSizes);

    if (fileSizes.empty()) {
        cout << "no files found\n";
        return 0;
    }

    map<uintmax_t, int> histogram = makeHistogram(fileSizes, binWidth);

    printHistogram(histogram, binWidth);
    saveHistogramToCsv(histogram, binWidth);

    return 0;
}