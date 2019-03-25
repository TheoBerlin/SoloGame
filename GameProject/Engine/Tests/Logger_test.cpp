#include <catch/catch.hpp>
#include <Engine/Utils/Logger.h>
#include <fstream>

TEST_CASE("Logger") {
    std::ofstream logFile;

    // Open and clear log file
    logFile.open(LOG_PATH, std::ofstream::out | std::ofstream::trunc);

    SECTION("Truncates logs file after first print") {
        logFile << "I should be truncated";
        logFile.flush();
        logFile.close();

        Logger::init();
        Logger::LOG_INFO("Hello");

        std::fstream fileReader(LOG_PATH);

        std::string readLine;
        std::getline(fileReader, readLine);

        REQUIRE(readLine != "I should be truncated");
    }
}
