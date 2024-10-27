#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <unordered_map>

struct Timing {
    int minutes;
    int seconds;
    int miliseconds;

    std::string toString() const {
        std::ostringstream oss;
        oss << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << "."
        << std::setw(3) << std::setfill('0') << miliseconds;
        return oss.str();
    }
};

struct IndividualRoundResult {
    int position;
    std::string name;
    Timing time;
    std::string constructor;
    
};

struct Driver {
    std::string driver;
    std::string manufacturer;
    int points;
};

struct Constructor {
    std::string manufacturer;
    int points;
    int numberOfWins = 0;
};

bool readRoundResultsFile(std::vector<Driver> &driversChampionship, std::vector<Constructor> &constructorsChampionship, char option);
IndividualRoundResult treatLine(const std::string &line);
Timing parseTime(const std::string& timeStr);
int driverPosition(std::vector<Driver> &driversChampionship, const std::string &driverName);
int constructorsPosition(std::vector<Constructor> &constructorsChampionship, const std::string &manufacturer);
void sortChampionships(std::vector<Driver> &driversChampionship, std::vector<Constructor> &constructorsChampionship);
void printDriversChampionship(const std::vector<Driver> &driversChampionship, int round);
void printConstructorsChampionship(const std::vector<Constructor> &constructorsChampionship, int round);
void updateStandings(std::vector<Driver> &driversChampionship,
                     std::vector<Constructor> &constructorsChampionship,
                     const IndividualRoundResult &result);

const int pointsAwarded [10] = {25, 18, 15, 12, 10, 8, 6, 4, 2};

const std::string roundsResultsFilePath = "roundsResults.txt";
const std::string classicRoundsResultsFilePath = "classicRoundsResults.txt";
std::unordered_map<std::string, std::string> driversAndTheirCarsMap;
std::unordered_map<std::string, std::vector<std::string>> driversWithDifferentCars;

int main() {
    std::vector<Driver> driversChampionship;
    std::vector<Constructor> constructorsChampionship;
    char option;
    std::cout << "Do you want to read the 2024 WRC file (W) or the Classics file (C): ";
    std::cin >> option;
    std::cout << "" << std::endl;

    std::cout << "Reading the previous times from the files..." << std::endl;
    if (readRoundResultsFile(driversChampionship, constructorsChampionship, option)) {
        std::cout << "Reading done. \n";
        if (driversWithDifferentCars.size() != 0) {
            std::cout << "The following drivers have used a different car: \n";
            for (const auto& pair : driversWithDifferentCars) {
                std::cout << "Driver: " << pair.first << "\nCars: ";
                for (const auto& car : pair.second) {
                    std::cout << car << ", "; // Print each car followed by a comma
                }
                std::cout << "\b\b " << std::endl; // Remove the last comma and space
            }
        }
    } else {
        std::cout << "There was an issue opening the file" << std::endl;
    }
}

bool readRoundResultsFile(std::vector<Driver> &driversChampionship, std::vector<Constructor> &constructorsChampionship, char option) {
    std::ifstream resultsFile;
    resultsFile.open(option == 'C' || option == 'c' ? classicRoundsResultsFilePath : roundsResultsFilePath);
    if (resultsFile.is_open()) {
        int rounds = 0;
        while (resultsFile) {
            std::string myline;
            std::getline (resultsFile, myline); // Round Line
            if (myline == "END") {
                break;
            } else {
                std::cout << "READING ROUND: " << myline << "\n";
                std::getline (resultsFile, myline); // ---- line
                bool exit = false;
                ++rounds;
                while (!exit) {
                    std::getline (resultsFile, myline);
                    if (myline == "---------------------") {
                        sortChampionships(driversChampionship, constructorsChampionship);
                        printDriversChampionship(driversChampionship, rounds);
                        printConstructorsChampionship(constructorsChampionship, rounds);
                        exit = true;
                    } else {
                        IndividualRoundResult result = treatLine(myline);
                        updateStandings(driversChampionship, constructorsChampionship, result);
                    }
                }
            }
        }
        return true;
    }
    else {
        std::cout << "Couldn't open the results file, is it on the same path as the .cpp? \n";
        return false;
    }
}

IndividualRoundResult treatLine(const std::string &input) {
    std::vector<std::string> words;
    std::stringstream ss(input);
    std::string token;
    // Split the input string by '#' and store each part in the tokens vector
    while (std::getline(ss, token, '#')) {
        words.push_back(token);
    }
    // Ensure we have the correct number of tokens
    if (words.size() != 4) {
        std::cout << "Line erroring out: \n";
        // Debug output to check how many tokens were found
        std::cout << "Token count: " << words.size() << " for line: " << input << std::endl;
        std::cout << std::endl;
        throw std::runtime_error("Invalid input format. Expected format is 'position#name#constructor#time'");
    }
    IndividualRoundResult result;
    result.position = std::stoi(words[0]);
    result.name = words[1];
    result.constructor = words[2];
    result.time = parseTime(words[3]);
    return result;
}

Timing parseTime(const std::string& timeStr) {
    Timing timing;
    char delim1, delim2; // Delimiters for ':' and '.'

    std::stringstream ss(timeStr);
    ss >> timing.minutes >> delim1 >> timing.seconds >> delim2 >> timing.miliseconds;

    // Error checking for correct delimiters
    if (ss.fail() || delim1 != ':' || delim2 != '.') {
        throw std::runtime_error("Invalid time format. Expected format is MM:SS.mmm");
    }

    return timing;
}

int driverPosition(std::vector<Driver> &driversChampionship, const std::string &driverName) {
    for (int i = 0; i < driversChampionship.size() ; ++i) {
        if (driversChampionship[i].driver == driverName) {
            return i;
        }
    }
    return -1;
}

int constructorsPosition(std::vector<Constructor> &constructorsChampionship, const std::string &manufacturer) {
    for (int i = 0; i < constructorsChampionship.size() ; ++i) {
        if (constructorsChampionship[i].manufacturer == manufacturer) {
            return i;
        }
    }
    return -1;
}

void sortChampionships(std::vector<Driver> &driversChampionship, std::vector<Constructor> &constructorsChampionship) {
    std::sort(driversChampionship.begin(), driversChampionship.end(),
              [](const Driver& a, const Driver& b) {
        return a.points > b.points; // Descending order
    });
    std::sort(constructorsChampionship.begin(), constructorsChampionship.end(),
              [](const Constructor& a, const Constructor& b) {
        return a.points > b.points; // Descending order
    });
}

void printDriversChampionship(const std::vector<Driver> &driversChampionship, int round) {
    std::cout << "\n######################################################################" << std::endl;
    std::cout << "############################# AFTER ROUND " << round << " ##########################" << std::endl;
    std::cout << "######################################################################\n" << std::endl;
    std::cout << std::left << std::setw(30) << "Driver"
              << std::setw(40) << "Constructor"
              << std::setw(10) << "Points" << std::endl;

    std::cout << std::string(30 + 40 + 10, '-') << std::endl; // Print a separator line

    // Display sorted drivers with formatted output
    for (const auto& driver : driversChampionship) {
        std::cout << std::left << std::setw(30) << driver.driver
                  << std::setw(40) << driver.manufacturer
                  << std::setw(10) << driver.points << std::endl; // Set a fixed width for points
    }
}

void printConstructorsChampionship(const std::vector<Constructor> &constructorsChampionship, int round) {
    std::cout << "\n" << std::endl;
    std::cout << std::left << std::setw(30) << "Manufacturer"
              << std::setw(40) << "Points"
              << std::setw(10) << "Wins" << std::endl;

    std::cout << std::string(30 + 40 + 10, '-') << std::endl; // Print a separator line
    
    // Display sorted manufacturers with formatted output
    for (const auto& constructor : constructorsChampionship) {
        std::cout << std::left << std::setw(30) << constructor.manufacturer
                  << std::setw(10) << constructor.points
                  << std::setw(10) << constructor.numberOfWins << std::endl; // Adjust spacing for wins
    }
}

void updateStandings(std::vector<Driver> &driversChampionship,
                     std::vector<Constructor> &constructorsChampionship,
                     const IndividualRoundResult &result) {
    int driverPositionInStandings = driverPosition(driversChampionship, result.name);
    int constructorPositionInStandings = constructorsPosition(constructorsChampionship, result.constructor);
    // Update the Drivers Championship
    if (driverPositionInStandings!= -1) {
        driversChampionship[driverPositionInStandings].points += result.position <= 10 ? pointsAwarded[result.position - 1] : 0;
        if (result.constructor != driversAndTheirCarsMap[result.name]) {
            std::cout << "Found one driver not using the same car all season. Inserting it in the list \n";
            driversWithDifferentCars[result.name].push_back(driversAndTheirCarsMap[result.name]);
            driversWithDifferentCars[result.name].push_back(result.constructor);
        }
    } else {
        // We store the car each driver is using
        driversAndTheirCarsMap[result.name] = result.constructor;
        driversChampionship.push_back(Driver(result.name, result.constructor, pointsAwarded[result.position-1]));
    }
    // Update the Constructors Championship
    if (constructorPositionInStandings!= -1) {
        constructorsChampionship[constructorPositionInStandings].points += result.position <= 10 ? pointsAwarded[result.position - 1] : 0;
        constructorsChampionship[constructorPositionInStandings].numberOfWins += result.position == 1;
    } else {
        constructorsChampionship.push_back(Constructor(result.constructor, pointsAwarded[result.position-1], result.position == 1));
    }
}
