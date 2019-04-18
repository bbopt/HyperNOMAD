//
//  hyperParameters.cpp
//  HyperNomad
//
//  Created by Christophe Tribes on 19-03-28.
//  Copyright © 2019 GERAD. All rights reserved.
//

#include "hyperParameters.hpp"


void trimLeft( NOMAD::Point & x )
{
    if ( x.size() == 1 )
    {
        x = NOMAD::Point();
        return;
    }

    NOMAD::Point xTrimmed ( static_cast<int>(x.size() - 1) );
    for ( size_t i=0 ; i < xTrimmed.size(); i++ )
    {
        xTrimmed[i]=x[i+1];
    }
    x = xTrimmed;
}

std::vector<NOMAD::bb_input_type> HyperParameters::getTypes() const
{
    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get dimension because the hyper parameters structure has not been expanded");
    }

    std::vector<NOMAD::bb_input_type> bbi;

    for ( auto const & block : _expandedHyperParameters )
    {
        std::vector<NOMAD::bb_input_type> blockBBI= block.getTypes( );
        bbi.insert(bbi.end(), std::begin(blockBBI), std::end(blockBBI));
    }
    return bbi;
}

size_t HyperParameters::getDimension() const
{
    size_t dim=0;

    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get dimension because the hyper parameters structure has not been expanded");
    }


    for ( auto const & block : _expandedHyperParameters )
    {
        dim += block.getDimension( );
    }

    return dim;
}


NOMAD::Point HyperParameters::getValues( ValueType t) const
{

    size_t dim = getDimension();

    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get values because the hyper parameters structure has not been expanded");
    }


    std::vector<NOMAD::Double> X0;
    for ( auto const & block : _expandedHyperParameters )
    {
        std::vector<NOMAD::Double> blockValues = block.getValues( t );
        X0.insert(X0.end(), std::begin(blockValues), std::end(blockValues));
    }

    if ( X0.size() != dim )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Dimensions are incompatible");
    }

    NOMAD::Point values( static_cast<int>(dim) );
    for ( size_t i = 0 ; i < X0.size() ; i++ )
    {
        values[i] = X0[i];
    }
    return values;
}

// MAYBE TODO
//std::vector<std::pair<size_t,NOMAD::Double>> HyperParameters::getFixedParams() const
//{
//    std::vector<std::pair<size_t,NOMAD::Double>> fixed;
//    return fixed;
//}

// Get the indices of fixed variables for expanded hyper parameters
std::vector<size_t> HyperParameters::getIndexFixedParams() const
{

    size_t current_index =0;
    std::vector<size_t> fixedParams;
    for ( auto aBlock : _expandedHyperParameters )
    {
        std::vector<size_t> blockFixedParams = aBlock.getIndexFixedParams( current_index );
        fixedParams.insert( fixedParams.begin() , blockFixedParams.begin() , blockFixedParams.end() );
    }
    return fixedParams;
}

std::vector<std::set<int>> HyperParameters:: getVariableGroupsIndices() const
{
    int current_index =0;
    std::vector<std::set<int>> indices;
    for ( auto aBlock : _expandedHyperParameters )
    {
        std::set<int> aGroupIndices;
        std::vector<NOMAD::bb_input_type> bbit = aBlock.getTypes ( );
        for ( size_t i = 0 ; i < bbit.size() ; i++, current_index++ )
        {
            // Fixed and categorical hyperparameters cannot be part of a Nomad group of variables
            //
            if ( bbit[i] != NOMAD::CATEGORICAL && ! aBlock.getHyperParameter(i).isFixed )
                aGroupIndices.insert(current_index);
        }
        if ( aGroupIndices.size() > 1 )
            indices.push_back( aGroupIndices );
    }
    return indices;
}



void HyperParameters::update( const NOMAD::Point & x )
{
    // Start over from baseHyperParameters that have not been expanded to full size
    _expandedHyperParameters = _baseHyperParameters;

    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the hyper parameters structure has not been expanded" );
    }

    NOMAD::Point xBlock (x);
    for ( auto & block : _expandedHyperParameters )
    {
        // Update the head of block parameter
        if ( block.headOfBlockHyperParameter.isDefined() )
        {
            block.headOfBlockHyperParameter.value = xBlock[0];
            trimLeft( xBlock );
        }

        // Expand the block structure from updtate the head value
        // Set the flags for dynamic fixed variables
        // update the associated parameters with xBlock value and trim xBlock for next block
        block.expandAssociatedParameters();
        block.setAssociatedParametersType();
        block.updateAssociatedParameters ( xBlock );
    }
    if ( xBlock.size() != 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the structure of hyper parameters is not consistent with the size of the point." );
    }

}

void HyperParameters::expand ()
{
    _expandedHyperParameters = _baseHyperParameters;
    for ( auto & block : _expandedHyperParameters )
    {
        block.expandAssociatedParameters();
    }
}


std::vector<HyperParameters> HyperParameters::getNeighboors( const NOMAD::Point & x )
{
    std::vector<HyperParameters> neighboors;

    // Update the HyperParameters from x --> _expandedHyperParameters is up to date
    update(x);

    for ( size_t i=0; i < _expandedHyperParameters.size() ; i++ )
    {

        // Get the neighboors for a given block of hyper parameters
        // The neighboors are expanded
        std::vector<HyperParametersBlock> nBlocks= _expandedHyperParameters[i].getNeighboorsOfBlock ( );

        // For each neighboor block: insert base blocks of hyper parameters before and after
        for ( auto & aNBlock : nBlocks )
        {

            std::vector<HyperParametersBlock> allBlocksForCompleteHyperParameters;
            // Push_back are used to fill the vector from begining to end with current blocks and neighboor block
            // All blocks are supposed to be expanded
            for ( size_t j= 0 ; j < _expandedHyperParameters.size() ;j++)
            {
                if ( i < j || i > j )
                    allBlocksForCompleteHyperParameters.push_back(_expandedHyperParameters[j]);
                else
                    allBlocksForCompleteHyperParameters.push_back( aNBlock );
            }
            neighboors.push_back( allBlocksForCompleteHyperParameters );
        }
    }
    return neighboors;
}

HyperParameters::HyperParameters ( const std::string & hyperParamFileName )
{
    initBlockStructureToDefault();
    
    registerSearchNames();
    
    if ( hyperParamFileName.empty() )
        std::cout << "WARNING: no hyper parameter file is provided, all values will be set to default." << std::endl;
    else
        read(hyperParamFileName);
    
    if ( _X0.is_defined() )
        update(_X0); // expansion is performed during update
    else
        // Need to expand the HyperParams to have all attributes set to default provided in block structure.
        expand();

    // Perform check on hyper parameters and set initial value for both base and expanded
    check();
}


// Sets some base hyper parameters value
// Also set some Nomad optimization parameters (MAX_BB_EVAL, X0, BB_EXE)
void HyperParameters::read (const std::string & hyperParamFileName )
{
    //
    // First read the file to set Nomad parameter entries
    //
    
    std::ifstream fin;
    std::string err = "Cannot read " + hyperParamFileName ;
    if ( NOMAD::check_read_file ( hyperParamFileName ) )
    {
        fin.open ( hyperParamFileName.c_str() );
        if ( !fin.fail() )
            err.clear();
    }
    if ( !err.empty() )
    {
        fin.close();
        throw NOMAD::Exception ( __FILE__ , __LINE__ , err );
    }
    
    // the set of entries:
    NOMAD::Parameter_Entries entries;
    
    // the file is read: fill the set 'entries' of Parameter_Entry:
    NOMAD::Parameter_Entry * pe;
    std::string              s;
    int line_number = 0;
    
    while ( fin.good() && !fin.eof() )
    {
        
        s.clear();
        
        getline ( fin , s );
        line_number++;
        
        NOMAD::string_vect_padding ( s );
        
        if ( !fin.fail() && !s.empty() )
        {
            pe = new NOMAD::Parameter_Entry ( s );
            pe->set_line(line_number);
            
            // First test on NOMAD parameters basic syntax
            // More test on parameters are done later
            if ( pe->is_ok() )
            {
                entries.insert ( pe ); // pe will be deleted by ~Parameter_Entries()
            }
            else
            {
                // NOMAD parameters basic syntax is not respected
                if ( pe->get_name() !="" )
                {
                    err = pe->get_name() + " does not respect parameters syntax." ;
                    throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() , err );
                }
                else // Case of a comment (erase silently)
                    delete pe;
            }
        }
    }
    // the file is closed:
    fin.close();
    
    //
    // Analyze the entries and set hyper parameters
    //
    int m;
    
    // BB_EXE:
    // -------
    {
        pe = entries.find ( "BB_EXE" );
        if ( pe )
        {
            if ( !pe->is_unique() )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                         "BB_EXE not unique" );
            
            m = pe->get_nb_values();
            
            if ( m == 1 )
                _bbEXE =  *pe->get_values().begin();
            else
            {
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                             "number of BB_EXE (>1)." );
            }
                pe->set_has_been_interpreted();
        }
    }
    
    // MAX_BB_EVAL:
    // ------------
    {
        int i;
        pe = entries.find ( "MAX_BB_EVAL" );
        if ( pe )
        {
            if ( !pe->is_unique() )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                         "MAX_BB_EVAL not unique" );
            if ( pe->get_nb_values() != 1 || !NOMAD::atoi (*(pe->get_values().begin()) , i) )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                         "MAX_BB_EVAL" );
            pe->set_has_been_interpreted();
            _maxBbEval = i;
        }
    }

    // X0:
    // ----------
    {
        std::vector<NOMAD::Double> tmp_x0;

        interpretPoint( ValueType::INITIAL_VALUE , tmp_x0 , &entries );
        
        _X0.reset( static_cast<int>(tmp_x0.size()) );
        size_t i = 0;
        for ( auto v : tmp_x0 )
            _X0[i++] = v;
    }
    
    
    
    //
    // SET SOME BASE (NOT EXPANDED) REGISTERED HYPER PARAMETER USING NAMES
    //
    {
        for ( const auto & searchName : _allSearchNames )
        {
            pe = entries.find ( searchName );
            if ( pe )
            {
                GenericHyperParameter * aHP = getHyperParameter( pe->get_name() );
                
                if ( aHP == nullptr )
                {
                    err = pe->get_name() + " is a registered hyper parameter but no hyper parameter can be obtained.";
                    throw NOMAD::Exception ( __FILE__ , __LINE__ , err );
                }
                
                if ( pe->get_nb_values() > 4 || pe->get_nb_values() < 1 )
                    throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                                "invalid number of values for "+searchName );

                
                std::list<std::string>::const_iterator it = pe->get_values().begin();
                NOMAD::Double                          v;
                
                // X0
                if ( !v.atof(*it) )
                    throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                                " cannot read value of "+searchName );
                aHP->value = v ;
                
                ++it;
                
                // Lower bound
                if ( it != pe->get_values().end() )
                {
                    if ( !v.atof(*it) )
                        throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                                    " cannot read value of "+searchName );
                    aHP->lowerBoundValue = v ;
                    
                    ++it;
                }
                
                // Upper bound
                if ( it != pe->get_values().end() )
                {
                    if ( !v.atof(*it) )
                        throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                                    " cannot read value of "+searchName );
                    aHP->upperBoundValue = v ;
                    
                    ++it;
                }
                
                // Is it fixed
                if ( it != pe->get_values().end() )
                {
                    if ( (*it).compare("FIXED") == 0 )
                    {
                        aHP->isFixed = true ;
                        aHP->fixedValue = aHP->value;
                    }
                    else
                        throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                                    " cannot read value of "+searchName );
                    ++it;
                }
                pe->set_has_been_interpreted();
            }
        }
    }
    
    
    
    pe = entries.find_non_interpreted();
    if ( pe )
    {
        err = pe->get_name() + " - unknown";
        throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() , err );
    }
    
}

void HyperParameters::interpretPoint( ValueType type,  std::vector<NOMAD::Double> & tmp , NOMAD::Parameter_Entries * entries ) const
{
    
    
    std::list<std::string>::const_iterator it;
    NOMAD::Double                          v;
    
    NOMAD::Parameter_Entry * pe ;
    
    if ( type == ValueType::INITIAL_VALUE )
        pe = entries->find ( "X0" );
    else if
        ( type == ValueType::LOWER_BOUND )
        pe = entries->find("LOWER_BOUND");
    else if
        ( type == ValueType::UPPER_BOUND )
        pe = entries->find("UPPER_BOUND");
    else
        throw NOMAD::Exception ( __FILE__ , __LINE__ , "Unknown value type" );
    
    if ( pe )
    {
        
        
        // Simpler version of reading NOMAD::Point taken from Nomad
        it = pe->get_values().begin();
        
        // Reading in the format X0 ( 1 2 3 4 5 )
        if ( *it != "(" && *it != "[" )
            throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                        "Point reading error: vector form must be within () or []" );
        
        ++it;
        while ( it !=pe->get_values().end() && *it != "]" && *it != ")"  )
        {
            if ( !v.atof(*it) )
                throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                            "Point reading error: cannot read values" );
            ++it;
            tmp.push_back( v );
        }
        
        if ( *it != "]" && *it != ")" )
            throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                        "Point reading error: vector form must be within () or []" );
        if ( tmp.size() == 0 )
            throw NOMAD::Parameters::Invalid_Parameter ( "In hyper parameter file " , pe->get_line() ,
                                                        "Point reading error: no values provided within [] or () " );
        pe->set_has_been_interpreted();
    }
    
}


HyperParameters::HyperParameters ( const std::vector<HyperParametersBlock> & hyperParamBlocks )
{

    // The vector of blocks is an expanded structure put into the object
    // This is equivalent to an assignement
    for ( const auto & block : hyperParamBlocks )
        _expandedHyperParameters.push_back( block );
}

void HyperParameters::registerSearchNames()
{
    _allSearchNames.clear();
    
    // Check base definition of hyper parameters
    for ( auto & aHyperParameterBlock : _baseHyperParameters )
    {
        aHyperParameterBlock.check();
        
        // Get all search names used in a block and verify that there is no duplicate name
        std::vector<std::string> blockSearchNames = aHyperParameterBlock.getSearchNames();
        _allSearchNames.insert(_allSearchNames.end(),blockSearchNames.begin(),blockSearchNames.end());
    }
    
    // Search duplicates based on the registered search name
    for ( std::vector<std::string>::iterator it=_allSearchNames.begin() ; it < _allSearchNames.end() ; it++ )
    {
        if ( std::find ( _allSearchNames.begin(), it , *it ) != it )
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: some hyper parameters in definition have duplicate names." );
    }
}

void HyperParameters::check( void )
{
   
    // Check base definition of hyper parameters
    for ( auto & aHyperParameterBlock : _baseHyperParameters )
    {
        aHyperParameterBlock.check();
    }

    // Check expanded
    for ( auto & aHyperParameterBlock : _expandedHyperParameters )
    {
        aHyperParameterBlock.check();
    }
}

HyperParameters::GenericHyperParameter * HyperParameters::getHyperParameter( const std::string & searchName )
{

    if ( std::find ( _allSearchNames.begin(), _allSearchNames.end() , searchName ) == _allSearchNames.end() )
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: cannot get an hyper parameters which name is " + searchName );
    
    GenericHyperParameter * aHP = nullptr;
    
    // Search base definition based on hyper parameter name
    for ( auto & aHyperParameterBlock : _baseHyperParameters )
    {
        aHP = aHyperParameterBlock.getHyperParameter( searchName );
        if ( aHP != nullptr && aHP->isDefined() )
            break;
    }
    return aHP;
}


void HyperParameters::initBlockStructureToDefault ( void )
{
    //
    // The block structure (with some default values) for Pytorch
    //


    // FIRST HYPER PARAMETERS BLOCK (Convolutionnal layers)
    GenericHyperParameter headOfBlock1={"NUM_CON_LAYERS","Number of convolutionnal layers",NOMAD::CATEGORICAL,2,0,100};

    GenericHyperParameter hp1={"NUM_OUTPUT_LAYERS","Number of output channels",NOMAD::INTEGER,6,1,1000, ReportValueType::COPY_VALUE};
    GenericHyperParameter hp2={"KERNELS","Kernel size",NOMAD::INTEGER,5,1,20, ReportValueType::COPY_VALUE};
    GenericHyperParameter hp3={"STRIDES","Stride",NOMAD::INTEGER,1,1,3, ReportValueType::COPY_VALUE};
    GenericHyperParameter hp4={"PADDINGS","Padding",NOMAD::INTEGER,0,0,2, ReportValueType::COPY_VALUE};
    GenericHyperParameter hp5={"DO_POOLS","Do a pooling",NOMAD::BINARY,0,0,1, ReportValueType::COPY_VALUE};
    GroupsOfAssociatedHyperParameters associatedHyperParameters1={{hp1,hp2,hp3,hp4,hp5}};

    HyperParametersBlock block1={"Convolutionnal layers",headOfBlock1, NeighborType::PLUS_ONE_MINUS_ONE_RIGHT, AssociatedHyperParametersType::MULTIPLE_TIMES, associatedHyperParameters1};


    // SECOND CATEGORICAL BLOCK (Full layers)
    GenericHyperParameter headOfBlock2={"NUM_FC_LAYERS","Number of full layers",NOMAD::CATEGORICAL,3,0,500};

    GenericHyperParameter hp6={"SIZE_NEXT_FC_LAYER","Size of a full layer",NOMAD::INTEGER,128,NOMAD::Double(),1000, ReportValueType::COPY_VALUE, FixedParameterType::IF_IN_LAST_GROUP, NOMAD::Double(10.0) };
    GroupsOfAssociatedHyperParameters associatedHyperParameters2={{hp6}};

    HyperParametersBlock block2={"Full  layers",headOfBlock2, NeighborType::PLUS_ONE_MINUS_ONE_LEFT, AssociatedHyperParametersType::MULTIPLE_TIMES, associatedHyperParameters2};

    // THIRD BLOCK (single regular parameter: batch size)
    GenericHyperParameter headOfBlock3={"BATCH_SIZE","Batch size",NOMAD::INTEGER,128,1,400,ReportValueType::NO_REPORT,FixedParameterType::NEVER};
    HyperParametersBlock block3={"Batch size",headOfBlock3,NeighborType::NONE,AssociatedHyperParametersType::ZERO_TIME,};

    // FOURTH CATEGORICAL BLOCK (Optimizer select)
    GenericHyperParameter headOfBlock4={"OPTIMIZER_CHOICE","Choice of optimizer",NOMAD::CATEGORICAL,3,1,4};

    GenericHyperParameter hp7={"OPT_PARAM_1","Learning rate",NOMAD::CONTINUOUS,0.1,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GenericHyperParameter hp8={"OPT_PARAM_2","Momentum",NOMAD::CONTINUOUS,0.9,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GenericHyperParameter hp9={"OPT_PARAM_3","Weight decay",NOMAD::CONTINUOUS,0.0005,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GenericHyperParameter hp10={"OPT_PARAM_4","Dampening",NOMAD::CONTINUOUS,0,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GroupsOfAssociatedHyperParameters associatedHyperParameters4={{hp7,hp8,hp9,hp10}};

    HyperParametersBlock block4={"Optimizer",headOfBlock4, NeighborType::LOOP_PLUS_ONE_RIGHT, AssociatedHyperParametersType::ONE_TIME, associatedHyperParameters4};

    // FIFTH BLOCK (single regular parameter: Dropout rate)
    GenericHyperParameter headOfBlock5={"DROPOUT_RATE","Dropout rate",NOMAD::CONTINUOUS,0.2,0,0.75};
    HyperParametersBlock block5={"Dropout rate",headOfBlock5, NeighborType::NONE, AssociatedHyperParametersType::ZERO_TIME,};

    // SIXTH BLOCK (single regular parameter: Activation function)
    GenericHyperParameter headOfBlock6={"ACTIVATION_FUNCTION","Activation function",NOMAD::INTEGER,1,1,3};
    HyperParametersBlock block6={"Activation function",headOfBlock6, NeighborType::NONE, AssociatedHyperParametersType::ZERO_TIME,};

    // ALL BASE HYPER PARAMETERS (NOT EXPANDED)
    _baseHyperParameters = {block1,block2,block3,block4,block5,block6};


    // Database name
    _databaseName = "MNIST";

    // BB
    _bbEXE = "$python ./pytorch_bb.py";

    // BB Output type
    _bbot={ NOMAD::OBJ };

    // Max BB eval
    _maxBbEval = 100;

    const double x0[]={10 , 1000 , 3  , 1  , 2 , 0 , 104 , 4 , 1 , 1 , 0 , 128 , 4 , 2 , 2 , 0 , 158 , 2 , 1 , 1 , 0 , 246 , 4 , 1 , 0 , 0 , 256 , 2 , 1 , 0 , 0 , 522 , 5 , 1 , 2 , 0 , 512 , 3 , 2 , 0 , 1 , 512 , 6 , 1 , 2 , 1 , 512 , 1 , 1 , 0 , 0 , 1 , 10 , 300 , 1 , 0.001,  0.9 , 0.0005 , 0 , 0.75, 1};
    size_t dim_x0 = sizeof(x0) / sizeof(double);
    _X0.reset ( static_cast<int>(dim_x0) );
    for ( int i=0 ; i < dim_x0 ; i++ )
        _X0[i]=x0[i];
}


/*void HyperParameters::initBlockStructureToDefault ( void )
{
    //
    // This default block structure corresponds to the PORTFOLIO problem
    //


    // FIRST HYPER PARAMETERS BLOCK (Assets)
    GenericHyperParameter headOfBlock1={"Number of assets",NOMAD::CATEGORICAL,1,1,3};

    GenericHyperParameter hp1={"Type of asset",NOMAD::INTEGER,1,0,2,COPY_VALUE};
    GenericHyperParameter hp2={"Value of an asset",NOMAD::CONTINUOUS,100,0,10000,COPY_VALUE};
    GroupsOfAssociatedHyperParameters associatedHyperParameters1={{hp1,hp2}};

    HyperParametersBlock block1={"Asset",headOfBlock1,PLUS_ONE_MINUS_ONE_RIGHT,MULTIPLE_TIMES,associatedHyperParameters1};

    // ALL BASE HYPER PARAMETERS (NOT EXPANDED)
    _baseHyperParameters = {block1};


    // Database name
    _databaseName = " ";

    // BB
    _bbEXE = "./bb.exe";

    // BB Output type
    _bbot={ NOMAD::EB , NOMAD::EB , NOMAD::OBJ };

    // Max BB eval
    _maxBbEval = 200;

    const double x0[]={1,1,100};
    size_t dim_x0 = sizeof(x0) / sizeof(double);
    _X0.reset ( static_cast<int>(dim_x0) );
    for ( int i=0 ; i < dim_x0 ; i++ )
        _X0[i]=x0[i];

} */



// Expand in block associated parameter by copying multiple time (if type allows) the existing parameters
void HyperParameters::HyperParametersBlock::expandAssociatedParameters()
{
    if ( groupsOfAssociatedHyperParameters.size() > 1 )
    {
        std::string err = "More than one group of associated hyper parameters. The expansion in block " +  name + " has already been done.";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( associatedParametersType == AssociatedHyperParametersType::ZERO_TIME && groupsOfAssociatedHyperParameters.size() > 0 )
    {
        std::string err = "There is a group of associated hyper parameters for "+name+" but the type is defined as ZERO_TIME.";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( headOfBlockHyperParameter.type != NOMAD::CATEGORICAL && neighborType != NeighborType::NONE )
    {
        std::string err = "Only categorical variables can have a neighboor type different than NONE. Head parameter " + headOfBlockHyperParameter.fullName + " is invalid ";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && neighborType == NeighborType::NONE )
    {
        std::string err = "Categorical variables must have a neighboor type different than NONE. Head parameter " + headOfBlockHyperParameter.fullName + " is invalid ";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    // The expansion is performed only for MULTIPLE_TIMES associated parameters
    if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && associatedParametersType == AssociatedHyperParametersType::MULTIPLE_TIMES ) // The head parameter of block has associated parameters
    {
        NOMAD::Double headValue = headOfBlockHyperParameter.value;

        if ( ! headValue.is_defined() || ! headValue.is_integer() || headValue < 0 )
        {
            std::string err = "The dimension of an hyper parameter block (head parameter " + headOfBlockHyperParameter.fullName + ") is invalid ";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }

        if ( groupsOfAssociatedHyperParameters.empty() )
        {
            std::string err = "Cannot copy associated parameters in block " +  name + " because it is empty ";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }

        std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = groupsOfAssociatedHyperParameters[0];
        // Expand the associated parameters by copying multiple times at the end of first group
        for ( size_t i= 1 ; i < headValue.round() ; i++ )
        {
            groupsOfAssociatedHyperParameters.push_back( tmpAssociatedHyperParameters ) ;
        }
    }
}

// Use to set the fixed flag for parameters that can be fixed or not depending if they are in the first or the last group of associated hyper parameters
void HyperParameters::HyperParametersBlock::setAssociatedParametersType()
{
    NOMAD::Double headValue = headOfBlockHyperParameter.value;
    if ( headValue.is_defined() && associatedParametersType == AssociatedHyperParametersType::MULTIPLE_TIMES && groupsOfAssociatedHyperParameters.size() != headValue.round() )
    {
        std::string err = "The number of groups of associated parameters is inconsistent with the head value for " + name ;
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    for ( auto & aGroupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : aGroupAHP )
        {
            // First group only
            if ( aHP.fixedParamType == FixedParameterType::IF_IN_FIRST_GROUP && &aGroupAHP == &groupsOfAssociatedHyperParameters.front() )
            {
                aHP.isFixed = true;
                if ( ! aHP.fixedValue.is_defined() )
                {
                    std::string err = "Cannot fix associated parameter " + name +" when in first group because the fixedValue is not provided";
                    throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
                }
                aHP.value = aHP.fixedValue;
            }
            // Last group only
            if ( aHP.fixedParamType == FixedParameterType::IF_IN_LAST_GROUP && &aGroupAHP == &groupsOfAssociatedHyperParameters.back() )
            {
                aHP.isFixed = true;
                if ( ! aHP.fixedValue.is_defined() )
                {
                    std::string err = "Cannot fix associated parameter " + name +" when in first group because the fixedValue is not provided";
                    throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
                }
                aHP.value = aHP.fixedValue;
            }

            if ( aHP.fixedParamType == FixedParameterType::ALWAYS )
            {
                aHP.isFixed = true;
                if ( ! aHP.fixedValue.is_defined() )
                {
                    std::string err = "Cannot fix associated parameter " + name +" when in first group because the fixedValue is not provided";
                    throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
                }
                aHP.value = aHP.fixedValue;
                continue;
            }
        }
    }
}

// Get an updated group of associated hyper parameters
std::vector<HyperParameters::GenericHyperParameter> HyperParameters::HyperParametersBlock::updateAssociatedParameters ( std::vector<HyperParameters::GenericHyperParameter> & fromGroup , bool isLastGroup , bool isFirstGroup ) const
{
    if ( isLastGroup && isFirstGroup && groupsOfAssociatedHyperParameters.size() > 1 )
    {
        std::string err = "Impossible to be in first and last group at the same time (" +  name +")";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( fromGroup.empty() )
    {
        std::string err = "Impossible to update associated parameters (" +  name +")";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    // By default all the values are copied ---> case aHP.reportValueType = COPY_VALUE
    std::vector<GenericHyperParameter> copiedGroup(fromGroup);

    // UPDATE THE COPIED GROUP ACCORDING TO CONSTRAINTS FOR FIXING / REPORTING VALUE
    for ( auto & aHP : copiedGroup )
    {
        if ( aHP.fixedParamType == FixedParameterType::ALWAYS && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = true;
            aHP.value = aHP.fixedValue;
            continue;
        }
        if ( aHP.fixedParamType == FixedParameterType::IF_IN_LAST_GROUP && isLastGroup && aHP.fixedValue.is_defined() )
        {
            aHP.value = aHP.fixedValue;
            aHP.isFixed = true ;
            continue;
        }
        if ( aHP.fixedParamType == FixedParameterType::IF_IN_FIRST_GROUP && isFirstGroup && aHP.fixedValue.is_defined() )
        {
            aHP.value = aHP.fixedValue;
            aHP.isFixed = true ;
            continue;
        }
        if ( aHP.reportValueType == ReportValueType::COPY_INITIAL_VALUE && aHP.initialValue.is_defined() )
        {
            aHP.value = aHP.initialValue;
            continue;
        }
    }

    // UPDATE THE FROM GROUP ACCORDING TO CONSTRAINTS FOR FIXING
    for ( auto & aHP : fromGroup )
    {
        if ( aHP.fixedParamType == FixedParameterType::ALWAYS && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = true;
            aHP.value = aHP.fixedValue;
            continue;
        }
        // The fromGroup is not the last anymore --> remove the fixed flag
        if ( aHP.fixedParamType == FixedParameterType::IF_IN_LAST_GROUP && isLastGroup && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = false ;
            continue;
        }
        if ( aHP.fixedParamType == FixedParameterType::IF_IN_FIRST_GROUP && isFirstGroup && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = false;
            continue;
        }
    }



    return copiedGroup;
}


void HyperParameters::HyperParametersBlock::expandAndUpdateAssociatedParametersWithConstraints ( void )
{
    if ( neighborType == NeighborType::NONE )
        return;

    if ( associatedParametersType == AssociatedHyperParametersType::MULTIPLE_TIMES ) // The head parameter of block has associated parameters
    {
        //
        // Expand the groups of associated parameters to head value without considering the upper bound
        //

        NOMAD::Double headValue = headOfBlockHyperParameter.value;
        NOMAD::Double headUpperBound = headOfBlockHyperParameter.upperBoundValue;

        if ( ! headValue.is_defined() || headValue > headUpperBound )
            return;

        // No need to expand. The number of groups >= the value in head of block
        if ( groupsOfAssociatedHyperParameters.size() >= headValue.round() )
            return;

        // Cases with PLUS ONE at RIGHT
        if ( neighborType == NeighborType::PLUS_ONE_MINUS_ONE_RIGHT || neighborType == NeighborType::LOOP_PLUS_ONE_RIGHT )
        {
            // Expand the associated parameters by copying multiple times at the end of first group
            while ( groupsOfAssociatedHyperParameters.size() < headValue.round() )
            {
                std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = updateAssociatedParameters ( groupsOfAssociatedHyperParameters.back() , true , false );
                groupsOfAssociatedHyperParameters.push_back( tmpAssociatedHyperParameters ) ;
            }
        }

        // Cases with PLUS ONE at LEFT
        if ( neighborType == NeighborType::PLUS_ONE_MINUS_ONE_LEFT || neighborType == NeighborType::LOOP_PLUS_ONE_LEFT )
        {
            // Expand the associated parameters by copying multiple times before first group
            while ( groupsOfAssociatedHyperParameters.size() < headValue.round() )
            {
                std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = updateAssociatedParameters ( groupsOfAssociatedHyperParameters[0] , false , true );
                groupsOfAssociatedHyperParameters.insert( groupsOfAssociatedHyperParameters.begin() , tmpAssociatedHyperParameters ) ;
            }
        }
    }
}

void HyperParameters::HyperParametersBlock::reduceAssociatedParametersWithConstraints ( void )
{
    if ( neighborType == NeighborType::NONE )
        return;

    if ( associatedParametersType == AssociatedHyperParametersType::MULTIPLE_TIMES ) // The head parameter of block has associated parameters
    {
        //
        // Reduce the groups of associated parameters to head value without considering the lower bound
        //


        NOMAD::Double headValue = headOfBlockHyperParameter.value;
        NOMAD::Double headLowerBound = headOfBlockHyperParameter.lowerBoundValue;

        if ( ! headValue.is_defined() || headValue < headLowerBound )
            return;


        // Cases with MINUS ONE at RIGHT
        if ( neighborType == NeighborType::PLUS_ONE_MINUS_ONE_RIGHT || neighborType == NeighborType::LOOP_MINUS_ONE_RIGHT )
        {
            // Reduce the associated parameters by erasing multiple times at the last group
            while ( groupsOfAssociatedHyperParameters.size() > headValue.round() )
            {
                groupsOfAssociatedHyperParameters.pop_back( ) ;
                updateAssociatedParameters ( groupsOfAssociatedHyperParameters.back() , true , false );
            }
        }

        // Cases with MINUS ONE at LEFT
        if ( neighborType == NeighborType::PLUS_ONE_MINUS_ONE_LEFT || neighborType == NeighborType::LOOP_MINUS_ONE_LEFT )
        {
            // Reduce the associated parameters by erasing multiple times the first group
            while ( groupsOfAssociatedHyperParameters.size() > headValue.round() )
            {
                groupsOfAssociatedHyperParameters.erase( groupsOfAssociatedHyperParameters.begin() ) ;
                if ( groupsOfAssociatedHyperParameters.size() > 0 )
                    updateAssociatedParameters ( groupsOfAssociatedHyperParameters[0] , false , true );
            }
        }
    }
}

size_t HyperParameters::HyperParametersBlock::getDimension ( ) const
{
    size_t s = 0;

    if ( headOfBlockHyperParameter.isDefined() )
        s++;

    for ( auto group : groupsOfAssociatedHyperParameters )
        s += group.size();

    return s;
}

void HyperParameters::HyperParametersBlock::updateAssociatedParameters( NOMAD::Point & x )
{


    // update associated parameters from x
    for ( auto & aGroupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : aGroupAHP )
        {
            if ( ! x.is_defined() )
            {
                std::string err = "Cannot update with a point of insufficient size. Block name: " + name + ".";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.type == NOMAD::CATEGORICAL )
            {
                std::string err = "Only head parameter can be of type CATEGORICAL. Invalid parameter " + aHP.fullName + ".";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            // Update the value
            aHP.value = x[0];

            // Trim x from the used value
            trimLeft( x );

            if ( aHP.isFixed && aHP.value != aHP.fixedValue )
            {
                std::string err = "Cannot set the fixed variable " + aHP.fullName + " to a value different than the provided fixedValue.";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
        }
    }

    // std::cout << x << std::endl;

}

void HyperParameters::HyperParametersBlock::check()
{

    if ( headOfBlockHyperParameter.isDefined() )
    {
        if ( ! headOfBlockHyperParameter.value.is_defined() )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.fullName + " has no value defined.";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        if (  headOfBlockHyperParameter.initialValue.is_defined() )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.fullName + " already has an init value. Initial value should be set only once";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }

        if (  headOfBlockHyperParameter.fixedParamType == FixedParameterType::ALWAYS && headOfBlockHyperParameter.fixedValue.is_defined() && headOfBlockHyperParameter.value != headOfBlockHyperParameter.fixedValue )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.fullName + " is always fixed but has an init value not consistent with the fixed value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        headOfBlockHyperParameter.initialValue = headOfBlockHyperParameter.value;

        if ( headOfBlockHyperParameter.lowerBoundValue.is_defined() && headOfBlockHyperParameter.value < headOfBlockHyperParameter.lowerBoundValue )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.fullName + " has a lower bound not consistent with the initial value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        if ( headOfBlockHyperParameter.upperBoundValue.is_defined() && headOfBlockHyperParameter.value > headOfBlockHyperParameter.upperBoundValue )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.fullName + " has an upper bound not consistent with the initial value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
    }

    for ( auto & groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : groupAHP )
        {
            if ( ! aHP.value.is_defined() )
            {
                std::string err = "The hyper parameter " + aHP.fullName + " has no value defined.";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if (  aHP.initialValue.is_defined() )
            {
                std::string err = "The hyper parameter " + aHP.fullName + " already has an init value. Initial value should be set only once";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.fixedParamType == FixedParameterType::ALWAYS && aHP.fixedValue.is_defined() && aHP.value != aHP.fixedValue )
            {
                std::string err = "The hyper parameter " + aHP.fullName + " has an init value not consistent with the fixed value";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
            if ( aHP.isFixed && ! aHP.fixedValue.is_defined() )
            {
                std::string err = "The hyper parameter " + aHP.fullName + " is fixed but has no fixed value";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.isFixed && aHP.fixedParamType == FixedParameterType::NEVER )
            {
                std::string err = "The hyper parameter " + aHP.fullName + " is fixed but has no fixed value";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.fixedParamType == FixedParameterType::ALWAYS )
            {
                aHP.isFixed = true;
                aHP.fixedValue = aHP.value;
            }

            if ( aHP.isFixed )
            {
                aHP.fixedParamType = FixedParameterType::ALWAYS;
                aHP.fixedValue = aHP.value;
            }

            // Set initial value from value given during initialization
            aHP.initialValue = aHP.value;
        }
    }
}

std::vector<size_t> HyperParameters::HyperParametersBlock:: getIndexFixedParams( size_t & current_index ) const
{
    std::vector<size_t> indices;
    if ( headOfBlockHyperParameter.isFixed )
        indices.push_back( current_index );
    current_index++;
    for ( auto groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto aHP : groupAHP )
        {
            if ( aHP.isFixed )
                indices.push_back( current_index );
            current_index++;
        }
    }
    return indices;
}

const HyperParameters::GenericHyperParameter & HyperParameters::HyperParametersBlock::getHyperParameter ( size_t index ) const
{
    if ( index == 0 )
        return headOfBlockHyperParameter;

    // We suppose that the groups may have a different size. So we simply go through all groups until reaching the targetd index
    size_t i = 1 ;  // The first hyperparameter of the group has index=1 (0 is for head)
    for ( auto groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( const auto & aHP : groupAHP )
        {
            if ( i == index )
                return aHP;
            i++;
        }
    }
    std::string err = "Not enough hyperparameters";
    throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
}


std::vector<NOMAD::bb_input_type> HyperParameters::HyperParametersBlock::getAssociatedTypes ( ) const
{
    std::vector<NOMAD::bb_input_type> bbi;
    for ( auto groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto aHP : groupAHP )
            bbi.push_back( aHP.type );
    }
    return bbi;
}

std::vector<NOMAD::Double> HyperParameters::HyperParametersBlock::getAssociatedValues ( ValueType t ) const
{
    std::vector<NOMAD::Double> values;
    for ( auto groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto aHP : groupAHP )
        {
            if ( t == ValueType::CURRENT_VALUE )
                values.push_back( aHP.value );
            else if ( t == ValueType::LOWER_BOUND )
                values.push_back( aHP.lowerBoundValue );
            else if ( t == ValueType::UPPER_BOUND )
                values.push_back ( aHP.upperBoundValue );
            else if ( t == ValueType::INITIAL_VALUE )
            {
                values.push_back ( aHP.initialValue );
            }
            else
            {
                std::string err = "The value type of " + name + "is not known ";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
        }
    }
    return values;
}

std::vector<NOMAD::bb_input_type> HyperParameters::HyperParametersBlock::getTypes( void ) const
{
    std::vector<NOMAD::bb_input_type> bbi{headOfBlockHyperParameter.type };
    std::vector<NOMAD::bb_input_type> bbiAssociatedParameters = getAssociatedTypes( );
    bbi.insert(bbi.end(),std::begin(bbiAssociatedParameters), std::end(bbiAssociatedParameters));

    return bbi;
}

std::vector<NOMAD::Double> HyperParameters::HyperParametersBlock::getValues( ValueType t ) const
{

    std::vector<NOMAD::Double> values;

    if ( headOfBlockHyperParameter.isDefined() )
    {
        if ( t == ValueType::CURRENT_VALUE )
            values.push_back( headOfBlockHyperParameter.value  );
        else if ( t == ValueType::LOWER_BOUND )
        {
            // If head is Categorical, the provided lower bound is for limiting the possible neighboors of a point. A categorical variable does not need bounds for optimization
            if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL )
                values.push_back( NOMAD::Double() );
            else
                values.push_back( headOfBlockHyperParameter.lowerBoundValue  );
        }
        else if ( t == ValueType::UPPER_BOUND )
        {
            // If head is Categorical, the provided upper bound is for limiting the possible neighboors of a point. A categorical variable does not need bounds for optimization
            if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL )
                values.push_back( NOMAD::Double() );
            else
                values.push_back ( headOfBlockHyperParameter.upperBoundValue  );
        }
        else
        {
            std::string err = "The value type of " + name + "is not known ";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
    }

    std::vector<NOMAD::Double> valuesAssociatedParameters = getAssociatedValues( t );
    values.insert(values.end(),std::begin(valuesAssociatedParameters), std::end(valuesAssociatedParameters));
    return values;
}


std::vector<HyperParameters::HyperParametersBlock> HyperParameters::HyperParametersBlock::getNeighboorsOfBlock( ) const
{
    std::vector<HyperParametersBlock> neighboorsOfBlock;

    // Neighboors are created only when the head of block is categorical
    if ( neighborType == NeighborType::NONE || headOfBlockHyperParameter.type != NOMAD::CATEGORICAL )
        return neighboorsOfBlock;

    // Make a Plus One and Minus copy of the block ( put in the neighboors only if options allow it )
    HyperParametersBlock newBlockPlusOne (*this);
    HyperParametersBlock newBlockMinusOne (*this);

    // Plus one on the head
    newBlockPlusOne.headOfBlockHyperParameter.value ++;
    newBlockMinusOne.headOfBlockHyperParameter.value --;

    // Perform a partial expansion (if not zero_time)
    newBlockPlusOne.expandAndUpdateAssociatedParametersWithConstraints();

    // Perform a partial reduction (if not zero_time)
    newBlockMinusOne.reduceAssociatedParametersWithConstraints();

    // Add plus one neighboor
    if ( neighborType == NeighborType::PLUS_ONE_MINUS_ONE_RIGHT || neighborType == NeighborType::PLUS_ONE_MINUS_ONE_LEFT )
    {
        // Add PlusOne only if not on upper bound
        if (  ! headOfBlockHyperParameter.upperBoundValue.is_defined() || ( headOfBlockHyperParameter.upperBoundValue.is_defined() && headOfBlockHyperParameter.value < headOfBlockHyperParameter.upperBoundValue ) )
        {
            // Add the new expanded block to the neighboors
            neighboorsOfBlock.push_back( newBlockPlusOne );
        }

        // Add MinusOne only if not on lower bound
        if (  ! headOfBlockHyperParameter.lowerBoundValue.is_defined() || ( headOfBlockHyperParameter.lowerBoundValue.is_defined() && headOfBlockHyperParameter.value > headOfBlockHyperParameter.lowerBoundValue ) )
        {
            // Add the new reduced block to the neighboors
            neighboorsOfBlock.push_back( newBlockMinusOne );
        }
    }

    if ( neighborType == NeighborType::LOOP_PLUS_ONE_LEFT
        || neighborType == NeighborType::LOOP_PLUS_ONE_RIGHT )
    {
        if ( ! headOfBlockHyperParameter.upperBoundValue.is_defined() || ( headOfBlockHyperParameter.upperBoundValue.is_defined() && headOfBlockHyperParameter.value < headOfBlockHyperParameter.upperBoundValue ) )
        {
            // Add the new expanded block to the neighboors
            neighboorsOfBlock.push_back( newBlockPlusOne );
        }
        else if ( headOfBlockHyperParameter.lowerBoundValue.is_defined() )
        {
            // Put the head hyper parameter value to the lower bound
            HyperParametersBlock newBlockOnLowerBound (*this);

            newBlockOnLowerBound.headOfBlockHyperParameter.value = newBlockOnLowerBound.headOfBlockHyperParameter.lowerBoundValue;

            newBlockOnLowerBound.reduceAssociatedParametersWithConstraints();

            neighboorsOfBlock.push_back( newBlockOnLowerBound );
        }
    }

    if ( neighborType == NeighborType::LOOP_MINUS_ONE_LEFT
        || neighborType == NeighborType::LOOP_MINUS_ONE_RIGHT )
    {
        if ( ! headOfBlockHyperParameter.lowerBoundValue.is_defined() || ( headOfBlockHyperParameter.lowerBoundValue.is_defined() && headOfBlockHyperParameter.value > headOfBlockHyperParameter.lowerBoundValue ) )
        {
            // Add the new expanded block to the neighboors
            neighboorsOfBlock.push_back( newBlockMinusOne );
        }
        else if ( headOfBlockHyperParameter.upperBoundValue.is_defined() )
        {
            // Put the head hyper parameter value to the upper bound
            HyperParametersBlock newBlockOnUpperBound (*this);

            newBlockOnUpperBound.headOfBlockHyperParameter.value = newBlockOnUpperBound.headOfBlockHyperParameter.lowerBoundValue;

            newBlockOnUpperBound.expandAndUpdateAssociatedParametersWithConstraints();

            neighboorsOfBlock.push_back( newBlockOnUpperBound );
        }
    }

    return neighboorsOfBlock;
}


std::vector<std::string> HyperParameters::HyperParametersBlock::getSearchNames() const
{
   
    std::vector<std::string> searchNames;
    searchNames.push_back( headOfBlockHyperParameter.searchName );
    
    if ( groupsOfAssociatedHyperParameters.size() > 0 )
    {
        for ( const auto & aAP : groupsOfAssociatedHyperParameters[0] )
            searchNames.push_back( aAP.searchName );
    }
    return searchNames;
}

HyperParameters::GenericHyperParameter * HyperParameters::HyperParametersBlock::getHyperParameter( const std::string & searchName )
{
    if ( headOfBlockHyperParameter.searchName.compare(searchName) == 0 )
        return &headOfBlockHyperParameter;

    if ( groupsOfAssociatedHyperParameters.size() > 0 )
    {
        for ( auto & aAP : groupsOfAssociatedHyperParameters[0] )
            if ( aAP.searchName.compare(searchName) == 0 )
                return &aAP;
    }
    return nullptr;
}

