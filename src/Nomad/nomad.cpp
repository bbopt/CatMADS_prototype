/**
 \file   nomad.cpp
 \brief  NOMAD main file
 \author Viviane Rochon Montplaisir
 \date   2017
 */

#include "../Nomad/nomad.hpp"

/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main (int argc, char ** argv)
{
    auto TheMainStep = std::make_unique<NOMAD::MainStep>();
    NOMAD::OutputQueue::getInstance()->setMaxStepLevel(14);
    std::string error;

    // Need at least parameters file.
    if (argc < 2)
    {
        NOMAD::OutputQueue::getInstance()->setDisplayDegree(1);
        TheMainStep->displayUsage(argv[0]);
    }

    else
    {

        if (argv[1][0] == '-')
        {
            // Options
            std::string option = argv[1];
            NOMAD::toupper(option);

            // Display usage if option '-u' has been specified
            if (option == "-U" || option == "-USAGE" || option == "--USAGE")
            {
                NOMAD::OutputQueue::getInstance()->setDisplayDegree(1);
                TheMainStep->displayUsage(argv[0]);
            }

            // Display info and usage if option '-i' has been specified
            else if (option == "-I" || option == "-INFO" || option == "--INFO")
            {
                NOMAD::OutputQueue::getInstance()->setDisplayDegree(2);
                TheMainStep->displayInfo();
                TheMainStep->displayUsage(argv[0]);
            }

            // Display version if option '-v' has been specified
            else if (option == "-V" || option == "-VERSION" || option == "--VERSION")
            {
                NOMAD::OutputQueue::getInstance()->setDisplayDegree(1);
                TheMainStep->displayVersion();
            }
            // Display help if option '-h' has been specified
            else if (option == "-H" || option == "-HELP" || option == "--HELP")
            {
                std::string helpSubject = "_____";  // Empty help subject displays a special message about the type of attributes and subjects (not all subjects)
                if (3 == argc)
                {
                    helpSubject = argv[2];
                    NOMAD::toupper( helpSubject );
                }
                TheMainStep->displayHelp ( helpSubject );
            }
            // Display developer help if option '-d' has been specified
            else if (option == "-D" || option == "-DEVHELP" || option == "--DEVHELP")
            {
                std::string helpSubject;
                if (3 == argc)
                {
                    helpSubject = argv[2];
                    NOMAD::toupper( helpSubject );
                    if ( helpSubject == "ALL" )
                        helpSubject = "";
                }
                TheMainStep->displayHelp(helpSubject, true);
            }
            // Display CSV documentation if option '-d' has been specified
            else if (option == "-CD" || option == "-CSVDOC" || option == "--CSVDOC")
            {
                TheMainStep->displayCSVDoc ( );
            }
            else
            {
                NOMAD::OutputQueue::getInstance()->setDisplayDegree(1);
                TheMainStep->AddOutputInfo("ERROR: Unrecognized option: " +option, NOMAD::OutputLevel::LEVEL_ERROR);
                TheMainStep->displayUsage(argv[0]);
            }
        }

        else
        {
            try
            {
                // Use first argument as parameters file.
                std::string paramfilename = argv[1];

                if (!NOMAD::checkReadFile(paramfilename))
                {
                    NOMAD::OutputQueue::getInstance()->setDisplayDegree(1);
                    error = std::string("ERROR: Could not read file \"") + argv[1] + "\"";
                    std::cerr << std::endl << error << std::endl << std::endl;
                    TheMainStep->displayUsage(argv[0]);
                }
                else
                {
                    TheMainStep->displayInfo();
                    
                    TheMainStep->setParamFileName(paramfilename);

                    // Reads parameters
                    TheMainStep->start();

                    // Creates the EvaluatorControl, Mads, and runs Mads.
                    TheMainStep->run();

                    TheMainStep->end();
                }
            }
            catch (NOMAD::Exception &e)
            {
                error = "ERROR: ";
                error += e.what();
                std::cerr << std::endl << error << std::endl << std::endl;
                NOMAD::OutputQueue::getInstance()->setDisplayDegree(0);
            }
        }
    }

    NOMAD::OutputQueue::Flush();

    return (error.empty()) ? EXIT_SUCCESS : EXIT_FAILURE;
}


