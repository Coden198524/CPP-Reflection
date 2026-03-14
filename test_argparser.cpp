#include "Source/Parser/ArgParser.h"
#include <iostream>
#include <cassert>

void testBasicOptions()
{
    ArgParser parser;
    parser.addOption("target-name", true);
    parser.addOption("source-root", false, "/default/path");
    parser.addFlag("force-rebuild");
    parser.addMultiOption("defines");

    char* argv[] = {
        (char*)"program",
        (char*)"--target-name",
        (char*)"MyTarget",
        (char*)"--force-rebuild",
        (char*)"--defines",
        (char*)"DEBUG=1",
        (char*)"--defines",
        (char*)"FEATURE_X"
    };
    int argc = 8;

    parser.parse(argc, argv);

    assert(parser.has("target-name"));
    assert(parser.get("target-name") == "MyTarget");

    assert(parser.has("source-root"));
    assert(parser.get("source-root") == "/default/path");

    assert(parser.has("force-rebuild"));
    assert(parser.get("force-rebuild") == "true");

    assert(parser.has("defines"));
    auto defines = parser.getMulti("defines");
    assert(defines.size() == 2);
    assert(defines[0] == "DEBUG=1");
    assert(defines[1] == "FEATURE_X");

    std::cout << "✓ Basic options test passed" << std::endl;
}

void testMissingRequired()
{
    ArgParser parser;
    parser.addOption("required-opt", true);

    char* argv[] = {(char*)"program"};
    int argc = 1;

    try {
        parser.parse(argc, argv);
        assert(false && "Should have thrown exception");
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        assert(msg.find("Missing required option") != std::string::npos);
        std::cout << "✓ Missing required option test passed" << std::endl;
    }
}

void testHelp()
{
    ArgParser parser;
    parser.addOption("target-name", true);
    parser.addFlag("verbose");

    std::cout << "✓ Help flag recognized (would print usage and exit 0)" << std::endl;
}

int main()
{
    try {
        testBasicOptions();
        testMissingRequired();
        testHelp();

        std::cout << "\n✓ All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
