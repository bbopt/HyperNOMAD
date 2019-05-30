/* ------------------------------------------------------------------------------*/
/*  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with NOMAD */
/*                                                                               */
/*                                                                               */
/*  This program is free software: you can redistribute it and/or modify it      */
/*  under the terms of the GNU Lesser General Public License as published by     */
/*  the Free Software Foundation, either version 3 of the License, or (at your   */
/*  option) any later version.                                                   */
/*                                                                               */
/*  This program is distributed in the hope that it will be useful, but WITHOUT  */
/*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        */
/*  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  */
/*  for more details.                                                            */
/*                                                                               */
/*  You should have received a copy of the GNU Lesser General Public License     */
/*  along with this program. If not, see <http://www.gnu.org/licenses/>.         */
/*                                                                               */
/*  You can find information on the NOMAD software at www.gerad.ca/nomad         */
/* ------------------------------------------------------------------------------*/



/*-------------------------------------------------------------------*/
/*            Example of a problem with categorical variables        */
/*-------------------------------------------------------------------*/
#include "nomad.hpp"
#include "hyperParameters.hpp"
#include <vector>
#include <memory>

#include "fileutils.hpp"

using namespace std;
using namespace NOMAD;

bool flagDisplayNeighboors = false;
std::string hyperNomadName ;
const std::string shortPytorchBBPath = "src" + std::string(dirSep) + "blackbox" + std::string(dirSep) + "pytorch_bb.py";
const std::string hyperNomadVersion = "1.0";


/*--------------------------------------------------*/
/*  user class to define categorical neighborhoods  */
/*--------------------------------------------------*/
class My_Extended_Poll : public Extended_Poll
{

private:

    // vector of signatures
    int _extended_poll_call;
    std::shared_ptr<HyperParameters> _hyperParameters;

public:

    // constructor:
    My_Extended_Poll ( Parameters & p , std::shared_ptr<HyperParameters> & hyperParameters ):
    Extended_Poll ( p ), _hyperParameters(std::move(hyperParameters))
    {
    }

    // destructor:
    virtual ~My_Extended_Poll ( void ) {}

    // construct the extended poll points:
    virtual void construct_extended_points ( const Eval_Point &);

};

void display_hyperusage( )
{
    cout << std::endl
    << "Run           : " << hyperNomadName << " hyperparameters_file"     << std::endl
    << "Info          : " << hyperNomadName << " -i"                       << std::endl
    << "Help          : " << hyperNomadName << " -h"                       << std::endl
    << "Version       : " << hyperNomadName << " -v"                       << std::endl
    << "Usage         : " << hyperNomadName << " -u"                       << std::endl
    << "Neighboors    : " << hyperNomadName << " -n hyperparameters_file"  << std::endl
    << std::endl;
}

void display_hyperversion()
{
    // Display nomad version
    cout << "--------------------------------------------------" << std::endl;
    cout << "  HyperNomad - version " << hyperNomadVersion << std::endl;
    cout << "--------------------------------------------------" << std::endl ;
    cout << "  Using Nomad version " << NOMAD::VERSION << " - www.gerad.ca/nomad" << std::endl ;
    cout << "--------------------------------------------------" << std::endl << std::endl ;
}

void display_hyperinfo()
{
    
    display_hyperversion();
    display_hyperusage();
}

void display_hyperhelp()
{
// TODO Put the complete help here
    display_hyperversion();
    display_hyperusage();
    
    std::cout << NOMAD::open_block("DATASET") << std::endl;
    std::cout << " Default: No default (must be provided)" << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    std::cout << NOMAD::open_block("MAX_BB_EVAL") << std::endl;
    std::cout << " Default: no default (must be provided)" << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    std::cout << NOMAD::open_block("BB_EXE") << std::endl;
    std::cout << " Default: $python $(HYPERNOMAD)/" + shortPytorchBBPath << std::endl;
    std::cout << NOMAD::close_block() << std::endl;

    std::cout << NOMAD::open_block("HYPER_DISPLAY") << std::endl;
    std::cout << " Default: 1 " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    std::cout << NOMAD::open_block("LH_ITERATION_SEARCH") << std::endl;
    std::cout << " Default: 0 " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;

    std::cout << NOMAD::open_block("X0") << std::endl;
    std::cout << " Default:  " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;

    std::cout << NOMAD::open_block("LOWER_BOUND") << std::endl;
    std::cout << " Default:  " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    std::cout << NOMAD::open_block("UPPER_BOUND") << std::endl;
    std::cout << " Default:  " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    std::cout << NOMAD::open_block("HYPERPARAM_NAME in {}") << std::endl;
    std::cout << " Default:  " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    std::cout << NOMAD::open_block("REMAINING_HYPERPARAMETERS") << std::endl;
    std::cout << " Default: VAR " << std::endl;
    std::cout << NOMAD::close_block() << std::endl;
    
    
}


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main ( int argc , char ** argv )
{
    
    // Detect hyper nomad name
    hyperNomadName = trimDir( argv[0] );
    if ( hyperNomadName.length() == 0 )
    {
        std::cerr << "Cannot determine hyperNomad name! " << endl;
        return 0;
    }
    
    // Detect hypernomad environment variable
    const char * hyperNomadPath = getenv ("HYPERNOMAD");
    if ( hyperNomadPath == nullptr )
    {
        std::cerr << "Cannot access HYPERNOMAD environment variable. Make sure to define it properly." << std::endl;
        return 0;
        
    }
        
    std::string pytorchBB = std::string(hyperNomadPath) + dirSep +shortPytorchBBPath;
    // The default Python script path is set relative to the HYPERNOMAD path
    // The script file are assessed for reading
    if ( ! checkAccess( pytorchBB ) )
    {
        std::cerr << "Cannot access to " << pytorchBB << ". Make sure to set the HYPERNOMAD environment variable properly." << std::endl;
        return 0;
        
    }


    
    std::string hyperParamFile="";
    if ( argc > 1 )
    {
        std::string mainArg = argv[1];
        if ( mainArg.substr(0,1).compare("-") == 0 )
        {
            switch ( mainArg.at(1))
            {
                case 'i':
                    display_hyperinfo();
                    return 0;
                    break;
                case 'h':
                    display_hyperhelp();
                    return 0;
                    break;
                case 'v':
                    display_hyperversion();
                    return 0;
                    break;
                case 'n':
                    flagDisplayNeighboors = true;
                    if ( argc == 3 )
                        hyperParamFile = argv[2];
                    else
                    {
                        display_hyperusage();
                        return 0;
                    }
                    break;
                default:
                    display_hyperusage();
                    return 0;
            }
        }
        else
            hyperParamFile = argv[1];
    }
    else
    {
        display_hyperusage();
    }

    // NOMAD initializations:
    begin ( argc , argv );
    
    // display:
    Display out ( cout );
    out.precision ( DISPLAY_PRECISION_STD );
    
    
    try
    {
        
        // parameters creation:
        Parameters p ( out );

        std::shared_ptr<HyperParameters> hyperParameters = std::make_shared<HyperParameters>(hyperParamFile , pytorchBB );

// For testing getNeighboors
        if ( flagDisplayNeighboors )
        {
            // Switch to full display
            hyperParameters->setHyperDisplay(3);
            
            NOMAD::Point X0 = hyperParameters->getValues( ValueType::CURRENT_VALUE);
            std::vector<HyperParameters> neighboors = hyperParameters->getNeighboors(X0);
            size_t index=0;
            for ( const auto & n : neighboors )
            {
                std::cout << std::endl << NOMAD::open_block("Neighboor #"+std::to_string(index)) ;
                n.display();
                std::cout << NOMAD::close_block() << std::endl << std::endl;
                index ++;
            }
            return 0;
        }
        
        
        p.set_DISPLAY_DEGREE( static_cast<int>( hyperParameters->getHyperDisplay() ) );

        p.set_DIMENSION( static_cast<int>(hyperParameters->getDimension()) );
        p.set_X0( hyperParameters->getValues( ValueType::CURRENT_VALUE) );
        p.set_BB_INPUT_TYPE( hyperParameters->getTypes() );
        p.set_LOWER_BOUND( hyperParameters->getValues( ValueType::LOWER_BOUND ) );
        p.set_UPPER_BOUND( hyperParameters->getValues( ValueType::UPPER_BOUND ) );

        std::vector<size_t> indexFixedParams = hyperParameters->getIndexFixedParams();
        for ( auto i : indexFixedParams )
            p.set_FIXED_VARIABLE( static_cast<int>(i) );

        // Each block forms a VARIABLE GROUP in Nomad
        std::vector<std::set<int>> variableGroupsIndices = hyperParameters->getVariableGroupsIndices();
        
        for ( auto aGroupIndices : variableGroupsIndices )
            p.set_VARIABLE_GROUP( aGroupIndices );

        p.set_BB_OUTPUT_TYPE ( hyperParameters->getBbOutputType() );
        p.set_BB_EXE( hyperParameters->getBB() );
        
        p.set_LH_SEARCH(0 , static_cast<int>( hyperParameters->getLhIterationSearch() ) );

        p.set_MAX_BB_EVAL( static_cast<int>( hyperParameters->getMaxBbEval()) );

        p.set_EXTENDED_POLL_TRIGGER ( 10 , false );
        
        p.set_DISPLAY_STATS("bbe ( sol ) obj");
        p.set_STATS_FILE("stats.txt","bbe ( sol ) obj");
        p.set_HISTORY_FILE("history.txt");

        // parameters validation:
        p.check();
        
        if ( hyperParameters->getHyperDisplay() > 2 )
        {
            display_hyperversion();
            
            std::cout << std::endl
                << NOMAD::open_block ( "Nomad parameters" ) << std::endl
                << p
                << NOMAD::close_block();
            
        }

        // extended poll:
        My_Extended_Poll ep ( p , hyperParameters );

        // algorithm creation and execution:
        Mads mads ( p , NULL , &ep , NULL , NULL );
        
        
        NOMAD::stop_type stopType = mads.run();
        
        if ( stopType == X0_FAIL )
            cerr << endl << "The starting point cannot be evaluated. Please verify that the Pytorch script is available. The default setting for bbExe (" << p.get_bb_exe().front() << " ) seems incorrect. Set _bbExe accordingly." << endl << endl;
        
    }
    catch ( exception & e ) {
        string error = string ( "HYPER NOMAD has been interrupted: " ) + e.what();
        if ( Slave::is_master() )
            cerr << endl << error << endl << endl;
    }


    Slave::stop_slaves ( out );
    end();

    return EXIT_SUCCESS;
}

/*--------------------------------------*/
/*  construct the extended poll points  */
/*      (categorical neighborhoods)     */
/*--------------------------------------*/
void My_Extended_Poll::construct_extended_points ( const Eval_Point & x)
{

    // Get the neighboors of the point (an update of the hyper parameters structure is performed)
    std::vector<HyperParameters> neighboors = _hyperParameters->getNeighboors(x);

    for ( auto & nHyperParameters : neighboors )
    {
        size_t nDim = nHyperParameters.getDimension();
        vector<bb_input_type> nBbit = nHyperParameters.getTypes();

        NOMAD::Point nLowerBound = nHyperParameters.getValues( ValueType::LOWER_BOUND );
        NOMAD::Point nUpperBound = nHyperParameters.getValues( ValueType::UPPER_BOUND );
        NOMAD::Point nX = nHyperParameters.getValues( ValueType::CURRENT_VALUE );

        // Create a parameter to obtain a signature for this neighboor
        NOMAD::Parameters nP ( _p.out() );
        nP.set_DIMENSION( static_cast<int>(nDim) );
        nP.set_X0 ( nX );
        nP.set_LOWER_BOUND( nLowerBound );
        nP.set_UPPER_BOUND( nUpperBound );

        nP.set_BB_INPUT_TYPE( nBbit );
        nP.set_MESH_TYPE( NOMAD::XMESH );  // Need to force set XMesh

        std::vector<size_t> indexFixed = nHyperParameters.getIndexFixedParams();
        for ( auto i : indexFixed )
            nP.set_FIXED_VARIABLE( static_cast<int>(i) );

        // Each block forms a NOMAD VARIABLE GROUP
        std::vector<std::set<int>> variableGroupsIndices = nHyperParameters.getVariableGroupsIndices();
        for ( auto aGroupIndices : variableGroupsIndices )
            nP.set_VARIABLE_GROUP( aGroupIndices );


        // Some parameters come from the original problem definition
        nP.set_BB_OUTPUT_TYPE( _p.get_bb_output_type() );
        nP.set_BB_EXE( _p.get_bb_exe() );
        // Check is need to create a valid signature
        nP.check();

        // The signature to be registered with the neighboor point
        add_extended_poll_point ( nX , *(nP.get_signature()) );
    }
}


