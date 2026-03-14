/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Main.cpp
** --------------------------------------------------------------------------*/

#include "Precompiled.h"

#include "ReflectionOptions.h"
#include "ReflectionParser.h"

#include "Switches.h"
#include "ArgParser.h"

#include <chrono>
#include <fstream>

void parse(const ArgParser &cmdLine);

int main(int argc, char *argv[])
{
    auto start = std::chrono::system_clock::now( );

    // parse command line
    try
    {
        // path to the executable
        auto exeDir = fs::path( argv[ 0 ] ).parent_path( );

        // set the working directory to the executable directory
        if (!exeDir.empty( ))
            fs::current_path( exeDir );

        ArgParser cmdLine;

        cmdLine.addOption( kSwitchTargetName, true );
        cmdLine.addOption( kSwitchSourceRoot, true );
        cmdLine.addOption( kSwitchInputSource, true );
        cmdLine.addOption( kSwitchModuleHeaderFile, true );
        cmdLine.addOption( kSwitchOutputModuleSource, true );
        cmdLine.addOption( kSwitchOutputModuleFileDirectory, true );
        cmdLine.addOption( kSwitchTemplateDirectory, false, "Templates/" );
        cmdLine.addOption( kSwitchPrecompiledHeader, false );
        cmdLine.addFlag( kSwitchForceRebuild );
        cmdLine.addFlag( kSwitchDisplayDiagnostics );
        cmdLine.addOption( kSwitchCompilerIncludes, false );
        cmdLine.addMultiOption( kSwitchCompilerDefines );

        cmdLine.parse( argc, argv );

        parse( cmdLine );
    }
    catch (std::exception &e)
    {
        utils::FatalError( e.what( ) );
    }
    catch (...)
    {
        utils::FatalError( "Unhandled exception occurred!" );
    }

    auto duration = std::chrono::system_clock::now( ) - start;

    std::cout << "Completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>( duration ).count( )
              << "ms" << std::endl;

    return EXIT_SUCCESS;
}

void parse(const ArgParser &cmdLine)
{
    ReflectionOptions options;

    options.forceRebuild =
        cmdLine.has( kSwitchForceRebuild );

    options.displayDiagnostics =
        cmdLine.has( kSwitchDisplayDiagnostics );

    options.targetName =
        cmdLine.get( kSwitchTargetName );

    options.sourceRoot =
        cmdLine.get( kSwitchSourceRoot );

    options.inputSourceFile =
        cmdLine.get( kSwitchInputSource );

    options.moduleHeaderFile =
        cmdLine.get( kSwitchModuleHeaderFile );

    options.outputModuleSource =
        cmdLine.get( kSwitchOutputModuleSource );

    options.outputModuleFileDirectory =
        cmdLine.get( kSwitchOutputModuleFileDirectory );

    // default arguments
    options.arguments =
    { {
        "-x",
        "c++",
        "-std=c++11",
        "-D__REFLECTION_PARSER__"
    } };

    if (cmdLine.has( kSwitchPrecompiledHeader ))
    {
        options.precompiledHeader =
            cmdLine.get( kSwitchPrecompiledHeader );
    }

    if (cmdLine.has( kSwitchCompilerIncludes ))
    {
        auto includes =
            cmdLine.get( kSwitchCompilerIncludes );

        std::ifstream includesFile( includes );

        std::string include;

        while (std::getline( includesFile, include ))
            options.arguments.emplace_back( "-I"+ include );
    }

    if (cmdLine.has( kSwitchCompilerDefines ))
    {
        auto defines =
            cmdLine.getMulti( kSwitchCompilerDefines );

        for (auto &define : defines)
            options.arguments.emplace_back( "-D"+ define );
    }

    options.templateDirectory =
        cmdLine.get( kSwitchTemplateDirectory );

    std::cout << std::endl;
    std::cout << "Parsing reflection data for target \""
              << options.targetName << "\""
              << std::endl;

    ReflectionParser parser( options );

    parser.Parse( );

    try
    {
        parser.GenerateFiles( );
    }
    catch (std::exception &e)
    {
        utils::FatalError( e.what( ) );
    }
}