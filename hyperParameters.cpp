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
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get dimension because the hyperparameters structure has not been expanded");
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
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get dimension because the hyperparameters structure has not been expanded");
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
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get values because the hyperparameters structure has not been expanded");
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

// Get the indices of fixed variables for expanded hyperparameters
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



void HyperParameters::updateFromBaseAndPerformExpansion( const NOMAD::Point & x , bool explicitSetX0 , bool explicitSetLowerBounds , bool explicitSetUpperBounds )
{
    // Start over from baseHyperParameters that have not been expanded to full size
    _expandedHyperParameters = _baseHyperParameters;
    
    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the hyperparameters structure has not been expanded" );
    }
    
    // Set undefined vectors
    NOMAD::Point xBlock;
    NOMAD::Point lbBlock;
    NOMAD::Point ubBlock;
    
    
    
    if ( explicitSetX0 )
        xBlock = x;
    
    // LowerBounds and UpperBounds are fixed
    if ( explicitSetLowerBounds )
        lbBlock = _lowerBound ;
    if ( explicitSetUpperBounds )
        ubBlock = _upperBound;
    
    for ( auto & block : _expandedHyperParameters )
    {
        // Update the head of block parameter
        if ( block.headOfBlockHyperParameter.isDefined() )
        {
            if ( explicitSetX0 )
            {
                block.headOfBlockHyperParameter.value = xBlock[0];
                trimLeft( xBlock );
            }
            if ( explicitSetLowerBounds )
            {
                block.headOfBlockHyperParameter.lowerBoundValue = lbBlock[0];
                trimLeft( lbBlock );
            }
            if ( explicitSetUpperBounds)
            {
                block.headOfBlockHyperParameter.upperBoundValue = ubBlock[0];
                trimLeft( ubBlock );
            }
        }
        else
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the head of block does not exist." );
        
        // Expand the block structure from the updated head value
        // Set the flags for dynamic fixed variables
        // update the associated parameters with xBlock value and trim xBlock for next block
        block.expandAssociatedParameters();
        block.updateAssociatedParameters ( xBlock ,lbBlock , ubBlock );
        
        
    }
    if ( xBlock.size() != 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the structure of hyperparameters is not consistent with the size of the point." );
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
    updateFromBaseAndPerformExpansion(x , true );
    
    for ( size_t i=0; i < _expandedHyperParameters.size() ; i++ )
    {
        
        // Get the neighboors for a given block of hyperparameters
        // The neighboors are expanded
        std::vector<HyperParametersBlock> nBlocks= _expandedHyperParameters[i].getNeighboorsOfBlock ( );
        
        // For each neighboor block: insert base blocks of hyperparameters before and after
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
        std::cout << "WARNING: no hyperparameter file is provided, all values will be set to default." << std::endl;
    else
    {
        read(hyperParamFileName);
        updateAndCheckAfterReading();
    }
    
    if ( ! _X0.is_defined() )
        throw NOMAD::Exception ( __FILE__ , __LINE__ , "X0 must have a default initialization" );
    
    // Update and expand block with X0, LOWER_BOUND and UPPER_BOUND
    // If not explicitely provided X0, LOWER_BOUND and UPPER_BOUND -> use what is in base and perform expansion
    updateFromBaseAndPerformExpansion( _X0 , _explicitSetX0, _explicitSetLowerBounds , _explicitSetUpperBounds );
    
    // Perform check on hyperparameters and set initial value for both base and expanded
    check();
    
    display();
}

void HyperParameters::HyperParametersBlock::display () const
{
    // Head of block
    std::cout << "\t Head of block ";
    headOfBlockHyperParameter.display();
    
    switch ( associatedParametersType )
    {
        case AssociatedHyperParametersType::ZERO_TIME :
            std::cout << "\t No associated hyperparameters " << std::endl;
            break;
        case AssociatedHyperParametersType::ONE_TIME :
            std::cout << "\t One time associated hyperparameters (always 1 group)" << std::endl;
            break;
        case AssociatedHyperParametersType::MULTIPLE_TIMES :
            std::cout << "\t Multiple times associated hyperparameters: " << groupsOfAssociatedHyperParameters.size() << " groups" << std::endl;
            break;
    }
    
    size_t index = 0;
    for ( const auto & aGAH : groupsOfAssociatedHyperParameters )
    {
        std::cout << "\t Group #" << index++ << std::endl;
        for ( const auto & aAP : aGAH )
        {
            std::cout << "\t\t " ;
            aAP.display();
        }
    }
}

void HyperParameters::GenericHyperParameter::display () const
{
    std::cout << searchName << " -> x0=" << value << ", lb=" << lowerBoundValue << ", ub=" << upperBoundValue << ((isFixed) ? ", is FIXED":", is VARIABLE") << std::endl;
}

void HyperParameters::display () const
{
    std::cout << "===================================================" << std::endl;
    std::cout << "             BLOCKS OF HYPERPARAMETERS             " << std::endl;
    std::cout << " Each block has a head hyperparameters and possibly" << std::endl;
    std::cout << " several groups of associated hyperameters."         << std::endl ;
    std::cout << "===================================================" << std::endl<< std::endl ;
    
    for ( auto & block : _expandedHyperParameters )
    {
        std::cout << "-------------------------"  << block.name << "----------------------------" << std::endl;
        block.display();
        std::cout << "-----------------------------------------------------------------------" << std::endl << std::endl;
    }
}



// Sets some base hyperparameters value
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
            pe->set_param_file( hyperParamFileName );
            
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
    // Analyze the entries and set hyperparameters
    //
    int m;
    
    // DATASET:
    // -------
    {
        pe = entries.find ( "DATASET" );
        if ( pe )
        {
            if ( !pe->is_unique() )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "DATASET not unique" );
            
            m = pe->get_nb_values();
            
            if ( m == 1 )
                _dataset =  *pe->get_values().begin();
            else
            {
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "DATASET must be provided only once." );
            }
            pe->set_has_been_interpreted();
        }
        else
            throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                        "DATASET must be provided." );
            
    }
    
    // NUMBER_OF_CLASSES:
    // -----------------
    {
        pe = entries.find ( "NUMBER_OF_CLASSES" );
        if ( pe )
        {
            if ( !pe->is_unique() )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "NUMBER_OF_CLASSES not unique" );
            
            m = pe->get_nb_values();
            
            if ( m == 1 )
            {
                if ( !_numberOfClasses.atof(*pe->get_values().begin()) )
                    throw NOMAD::Parameters::Invalid_Parameter ( "In hyperparameter file " , pe->get_line() ,                                                                " cannot read value of NUMBER_OF_CLASSES" );
                _explicitelyProvidedNumberOfClasses = true;
            }
            else
            {
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "NUMBER_OF_CLASSES has too many arguments (>1)." );
            }
            pe->set_has_been_interpreted();
        }
    }
    
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
    // THIS CAN BE SUPERSEDED BY SETTING ON SPECIFIC HYPERPARAM --> see updateBaseAndExpand
    // ----------
    _explicitSetX0 = false;
    interpretX0( &entries );
    
    
    //
    //  NOT AVAILABLE FOR THE MOMENT
    //
    //    // FIXED_VARIABLES: (same as NOMAD)
    //    // THIS CAN BE SUPERSEDED BY SETTING FIXED OR VAR ON SPECIFIC HYPERPARAM --> see check
    //    // --------------------
    //    {
    //        _fixedVariables.resize ( _X0.size() );
    //
    //        interpretBoundsAndFixed( "FIXED_VARIABLE" , entries , _fixedVariables );
    //
    //
    //    }
    
    // LOWER_BOUND: (same as NOMAD)
    // THIS CAN BE SUPERSEDED BY SETTING BOUND ON SPECIFIC HYPERPARAM --> see check
    // --------------------
    {
        _explicitSetLowerBounds  = false;
        _lowerBound.resize ( _X0.size() );
        interpretBoundsAndFixed( "LOWER_BOUND" , entries , _lowerBound );
        
        
    }
    
    // UPPER_BOUND: (same as NOMAD)
    // THIS CAN BE SUPERSEDED BY SETTING BOUND ON SPECIFIC HYPERPARAM --> see check
    // --------------------
    {
        _explicitSetUpperBounds  = false;
        _upperBound.resize ( _X0.size() );
        interpretBoundsAndFixed( "UPPER_BOUND" , entries , _upperBound );
        
    }
    
        
    //
    // SET SOME BASE (NOT EXPANDED) REGISTERED HYPER PARAMETER USING NAMES
    //
    {
        bool alreadyDisplayedMessage = false;
        for ( const auto & searchName : _allSearchNames )
        {
            pe = entries.find ( searchName );
            if ( pe )
            {
                if ( !alreadyDisplayedMessage && ( _explicitSetLowerBounds || _explicitSetUpperBounds || _explicitSetX0 ) )
                {
                    alreadyDisplayedMessage  = true;
                    std::cout << "===============================================================" << std::endl;
                    std::cout << "WARNING: hyperparameters explicitely set by name are superseded" << std::endl;
                    std::cout << "by settings done using X0, LOWER_BOUND and UPPER_BOUND." << std::endl;
                    std::cout << "===============================================================" << std::endl << std::endl;
                }
                    
                GenericHyperParameter * aHP = getHyperParameter( pe->get_name() );
                
                if ( aHP == nullptr )
                {
                    err = pe->get_name() + " is a registered hyperparameter but no hyperparameter can be obtained.";
                    throw NOMAD::Exception ( __FILE__ , __LINE__ , err );
                }
                
                if ( pe->get_nb_values() > 4 || pe->get_nb_values() < 1 )
                    throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                                "invalid number of values for "+searchName );
                
                
                std::list<std::string>::const_iterator it = pe->get_values().begin();
                NOMAD::Double                          v;
                
                // X0 per hyperparam
                if ( !v.atof(*it) )
                    throw NOMAD::Parameters::Invalid_Parameter (  hyperParamFileName  , pe->get_line() ,
                                                                " cannot read value of "+searchName );
                if ( v.is_defined() )
                    aHP->value = v ;
                
                ++it;
                
                // Lower bound per hyperparam
                // SUPERSEDED WHEN SETTING LOWER_BOUND
                if ( it != pe->get_values().end() )
                {
                    if ( !v.atof(*it) ) // Undefined (-) is ok
                        throw NOMAD::Parameters::Invalid_Parameter (  hyperParamFileName  , pe->get_line() ,
                                                                    " cannot read value of "+searchName );
                    aHP->lowerBoundValue = v ;
                    
                    ++it;
                }
                
                // Upper bound per hyperparam
                // SUPERSEDED WHEN SETTING UPPER_BOUND
                if ( it != pe->get_values().end() )
                {
                    if ( !v.atof(*it) ) // Undefined (-) is ok
                        throw NOMAD::Parameters::Invalid_Parameter (  hyperParamFileName  , pe->get_line() ,
                                                                    " cannot read value of "+searchName );
                    aHP->upperBoundValue = v ;
                    
                    ++it;
                }
                
                // Is it a fixed or variable hyperparam
                // THE FIXED/VAR FLAG IS NOT SUPERSEDED
                if ( it != pe->get_values().end() )
                {
                    if ( (*it).compare("FIXED") == 0 )
                        aHP->isFixed = true ;
                    else if ( (*it).compare("VAR") == 0 )
                        aHP->isFixed = false ;
                    else
                        throw NOMAD::Parameters::Invalid_Parameter (  hyperParamFileName  , pe->get_line() ,
                                                                    " cannot read value of "+searchName );
                    ++it;
                }
                pe->set_has_been_interpreted();
                
                // This hyperparameter is set by its name. This flag is used when setting the remaining parameters (see below)
                aHP->settingByName = true;
            }
        }
    }
    
    // REMAINING_VARIABLES:
    // ------------
    {
        pe = entries.find ( "REMAINING_HYPERPARAMETERS" );
        if ( pe )
        {
            if ( !pe->is_unique() )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "REMAINING_HYPERPARAMETERS not unique" );
            if ( pe->get_nb_values() != 1 )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "REMAINING_HYPERPARAMETERS FIXED/VAR" );
            
            bool fixed = false;
            if ( pe->get_values().begin()->compare("FIXED") == 0 )
                fixed = true;
            else if ( pe->get_values().begin()->compare("VAR") != 0 )
                throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() ,
                                                            "REMAINING_HYPERPARAMETERS FIXED/VAR" );
            
            for ( auto & block : _baseHyperParameters )
            {
                // Update the head of block parameter
                if ( block.headOfBlockHyperParameter.isDefined() )
                {
                    // The number of classes is always fixed and CANNOT be changed
                    if ( block.headOfBlockHyperParameter.searchName.compare("NUMBER_OF_CLASSES") == 0 )
                        continue;
                    
                    if ( ! block.headOfBlockHyperParameter.settingByName )
                        block.headOfBlockHyperParameter.isFixed = fixed;
                }
                else
                    throw NOMAD::Exception( __FILE__,__LINE__,"HyperParameters: Undefined head of block hyperparameter.");
                
                // The group of associated parameters (base before expansion) for this block
                if ( block.groupsOfAssociatedHyperParameters.size() > 0 )
                {
                    for ( auto & aAHP : block.groupsOfAssociatedHyperParameters[0] )
                    {
                        // Update the first group of block associated parameters
                        if ( aAHP.isDefined() )
                        {
                            if ( ! aAHP.settingByName )
                                aAHP.isFixed = fixed;
                        }
                        else
                            throw NOMAD::Exception( __FILE__,__LINE__,"HyperParameters:  Undefined associated hyperparameters for this block.");
                    }
                }
            }
            pe->set_has_been_interpreted();
        }
    }
    
    pe = entries.find_non_interpreted();
    if ( pe )
    {
        err = pe->get_name() + " - unknown";
        throw NOMAD::Parameters::Invalid_Parameter ( hyperParamFileName , pe->get_line() , err );
    }
    
}


void HyperParameters::updateAndCheckAfterReading ( void )
{
    if ( _dataset.empty() )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: The DATASET name must be provided" );
    }
    
    
    // Check that the dataset name is registered
    if ( _datasetAndNumberOfClasses.find( _dataset ) == _datasetAndNumberOfClasses.end() )
    {
        std::cout << "WARNING: the DATASET name " << _dataset << " is not registered in the current list of available dataset. This requires to modify the blackbox." << std::endl;
        if ( ! _explicitelyProvidedNumberOfClasses )
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: If a DATASET not in the registered list is provided, the NUMBER_OF_CLASSES must be explicitely provided in the hyperparam.txt file." );
    }
    
    // Set the number of classes
    if ( _explicitelyProvidedNumberOfClasses )
    {
        // The registered default number of classes for the dataset is NOT used
        // A value was explicitely provided ---> manage inconsistancy
        if ( _datasetAndNumberOfClasses.find( _dataset )->second != _numberOfClasses )
        {
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: the default number of classes for the dataset is incompatible with the provided value using NUMBER_OF_CLASSES.");
        }
    }
    else
    {  // The registered default number of classes for the dataset is used
        _numberOfClasses = _datasetAndNumberOfClasses.find( _dataset )->second ;
    }
    
    // Complete the line for the blackbox with the dataset name (ex.: python pytorch_bb.py MNIST)
    _bbEXE += " " + _dataset;
    
}

void HyperParameters::interpretBoundsAndFixed ( const std::string & paramName , const NOMAD::Parameter_Entries & entries , NOMAD::Point & param )
{
    
    const std::string invalidFormatErr = "Invalid format for " + paramName;
    
    NOMAD::Parameter_Entry                 * pe = entries.find ( paramName );
    if ( !pe )
        return;
    
    std::string                              err;
    if ( !pe->is_unique() )
    {
        err = paramName + " not unique";
        throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), err );
    }
    
    
    
    std::list<std::string>::const_iterator   it;
    int                                      i, j, k;
    NOMAD::Double                            v;
    
    
    int dimension = param.size();
    
    if ( dimension == 0 )
    {
        err = paramName + " must have a dimension";
        throw NOMAD::Exception ( __FILE__ , __LINE__, err  );
    }
    
    if ( paramName.compare("LOWER_BOUND") == 0 )
        _explicitSetLowerBounds = true;
    
    if ( paramName.compare("UPPER_BOUND") == 0 )
        _explicitSetUpperBounds = true;
    
    
    // just one index or *:
    if ( pe->get_nb_values() == 1 )
    {
        
        if ( paramName.compare("FIXED_VARIABLE") != 0 )
        {
            err = "Cannot set " + paramName +" using a single value or * ";
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), err );
        }
        
        // special case for FIXED_VARIABLE without value
        // (the value will be taken from x0, if unique):
        if ( isdigit ( (*pe->get_values().begin())[0] ) ||
            *pe->get_values().begin() == "*"               )
        {
            
            if ( !NOMAD::string_to_index_range ( *pe->get_values().begin() ,
                                                i                         ,
                                                j                         ,
                                                & dimension  )              )
                throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), invalidFormatErr );
            
            for ( k = i ; k <= j ; ++k )
                param[k] = _X0[k] ;
        }
        else
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), invalidFormatErr );
        
    }
    
    // vector form: all values on one row:
    else if ( pe->get_nb_values() == dimension + 2 )
    {
        
        if ( !pe->is_unique() )
        {
            err = paramName + " has been given in vector form with [] or () and is not unique";
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() , err );
        }
        
        it = pe->get_values().begin();
        
        if ( *it != "[" && *it != "(" )
        {
            err = paramName + " in vector form with () or []";
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() , err );
        }
        
        ++it;
        for ( k = 0 ; k < dimension ; ++k )
        {
            if ( !v.atof(*it) )
                throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), invalidFormatErr );
            
            ++it;
            param[k] = v;
        }
        
        if ( *it != "]" && *it != ")" )
        {
            err = paramName + " error in vector form with () or []";
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() , err );
        }
    }
    
    // indexed values:
    else
    {
        
        if ( pe->get_nb_values() != 2 )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), invalidFormatErr );
        
        it = pe->get_values().begin();
        if ( !NOMAD::string_to_index_range ( *it , i , j , &dimension ) )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), invalidFormatErr );
        ++it;
        if ( !v.atof(*it) )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line(), invalidFormatErr );
        
        for ( k = j ; k >= i ; --k )
            param[k] =v;
    }
    pe->set_has_been_interpreted();
}


void HyperParameters::interpretX0( NOMAD::Parameter_Entries * entries )
{
    
    std::list<std::string>::const_iterator it;
    NOMAD::Double                          v;
    
    NOMAD::Parameter_Entry * pe ;
    
    pe = entries->find ( "X0" );
    
    if ( pe )
    {
        _explicitSetX0 = true;
        
        std::vector<NOMAD::Double> tmpX0;
        
        // Simpler version of reading NOMAD::Point taken from Nomad
        it = pe->get_values().begin();
        
        // Reading in the format X0 ( 1 2 3 4 5 )
        if ( *it != "(" && *it != "[" )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() ,
                                                        "Point X0 reading error: vector form must be within () or []" );
        
        ++it;
        while ( it !=pe->get_values().end() && *it != "]" && *it != ")"  )
        {
            if ( !v.atof(*it) )
                throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file(), pe->get_line() ,
                                                            "Point X0 reading error: cannot read values" );
            ++it;
            tmpX0.push_back( v );
        }
        
        if ( *it != "]" && *it != ")" )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() ,
                                                        "Point X0 reading error: vector form must be within () or []" );
        if ( tmpX0.size() == 0 )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() ,
                                                        "Point X0 reading error: no values provided within [] or () " );
        pe->set_has_been_interpreted();
        
        
        // Set X0
        _X0.reset( static_cast<int>(tmpX0.size()) );
        size_t i = 0;
        for ( auto v : tmpX0 )
            _X0[i++] = v;
        
        
        pe = pe->get_next();
        
        if ( pe )
            throw NOMAD::Parameters::Invalid_Parameter ( pe->get_param_file() , pe->get_line() ,
                                                        "Point X0 reading error: multiplie definition" );
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
    
    // get search name from base definition of hyperparameters
    for ( auto & aHyperParameterBlock : _baseHyperParameters )
    {
        // Get all search names used in a block and verify that there is no duplicate name
        std::vector<std::string> blockSearchNames = aHyperParameterBlock.getSearchNames();
        _allSearchNames.insert(_allSearchNames.end(),blockSearchNames.begin(),blockSearchNames.end());
    }
    
    // Search duplicates based on the registered search name
    for ( std::vector<std::string>::iterator it=_allSearchNames.begin() ; it < _allSearchNames.end() ; it++ )
    {
        if ( std::find ( _allSearchNames.begin(), it , *it ) != it )
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: some hyperparameters in definition have duplicate names." );
    }
}

void HyperParameters::check( void )
{
    // Check base definition of hyperparameters
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
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: cannot get an hyperparameters which name is " + searchName );
    
    GenericHyperParameter * aHP = nullptr;
    
    // Search base definition based on hyperparameter name
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
    
    // Pytorch dataset available by default --> link with number of classes
    _datasetAndNumberOfClasses = { {"MNIST",NOMAD::Double(10)},{"Fashion-MNIST",NOMAD::Double(10)},{"EMNIST",NOMAD::Double(10)}, {"KMNIST",NOMAD::Double(10)} , {"CIFAR10",NOMAD::Double(10)} , {"CIFAR100",NOMAD::Double(100)} , {"STL10",NOMAD::Double(10)}, {"SVHN",NOMAD::Double(10)} };
    
    // dataset name no default
    _dataset = "";
    _numberOfClasses = 0;
    
    // At this point the number of classes is NOT explicitely provided (maybe when done when reading the hyperparameter file) and the value registered is used
    _explicitelyProvidedNumberOfClasses = false;
    
    
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
    GenericHyperParameter headOfBlock2={"NUM_FC_LAYERS","Number of modifyable full layers",NOMAD::CATEGORICAL,2,0,500};
    
    GenericHyperParameter hp6={"SIZE_FC_LAYER","Size of a full layer",NOMAD::INTEGER,128,1,1000, ReportValueType::COPY_VALUE };
    
    GroupsOfAssociatedHyperParameters associatedHyperParameters2={{hp6}};
    
    HyperParametersBlock block2={"Full layers",headOfBlock2, NeighborType::PLUS_ONE_MINUS_ONE_LEFT, AssociatedHyperParametersType::MULTIPLE_TIMES, associatedHyperParameters2};
    
    
    // THIRD BLOCK (single regular parameter: number of classes ALWAYS FIXED)
    GenericHyperParameter headOfBlock3={"NUMBER_OF_CLASSES","Number of classes",NOMAD::INTEGER,_numberOfClasses,NOMAD::Double(),NOMAD::Double(),ReportValueType::NO_REPORT, true /*isFixed=true*/};
    HyperParametersBlock block3={"Number of classes",headOfBlock3,NeighborType::NONE,AssociatedHyperParametersType::ZERO_TIME,};
    
    
    // FOURTH BLOCK (single regular parameter: batch size)
    GenericHyperParameter headOfBlock4={"BATCH_SIZE","Batch size",NOMAD::INTEGER,128,1,400};
    HyperParametersBlock block4={"Batch size",headOfBlock4,NeighborType::NONE,AssociatedHyperParametersType::ZERO_TIME,};
    
    // FIFTH CATEGORICAL BLOCK (Optimizer select)
    GenericHyperParameter headOfBlock5={"OPTIMIZER_CHOICE","Choice of optimizer",NOMAD::CATEGORICAL,3,1,4};
    
    GenericHyperParameter hp7={"OPT_PARAM_1","Learning rate",NOMAD::CONTINUOUS,0.1,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GenericHyperParameter hp8={"OPT_PARAM_2","Momentum",NOMAD::CONTINUOUS,0.9,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GenericHyperParameter hp9={"OPT_PARAM_3","Weight decay",NOMAD::CONTINUOUS,0.0005,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GenericHyperParameter hp10={"OPT_PARAM_4","Dampening",NOMAD::CONTINUOUS,0,0,1, ReportValueType::COPY_INITIAL_VALUE};
    GroupsOfAssociatedHyperParameters associatedHyperParameters5={{hp7,hp8,hp9,hp10}};
    
    HyperParametersBlock block5={"Optimizer",headOfBlock5, NeighborType::LOOP_PLUS_ONE_RIGHT, AssociatedHyperParametersType::ONE_TIME, associatedHyperParameters5};
    
    // SIXTH BLOCK (single regular parameter: Dropout rate)
    GenericHyperParameter headOfBlock6={"DROPOUT_RATE","Dropout rate",NOMAD::CONTINUOUS,0.2,0,0.75};
    HyperParametersBlock block6={"Dropout rate",headOfBlock6, NeighborType::NONE, AssociatedHyperParametersType::ZERO_TIME,};
    
    // SIXTH BLOCK (single regular parameter: Activation function)
    GenericHyperParameter headOfBlock7={"ACTIVATION_FUNCTION","Activation function",NOMAD::INTEGER,1,1,3};
    HyperParametersBlock block7={"Activation function",headOfBlock7, NeighborType::NONE, AssociatedHyperParametersType::ZERO_TIME,};
    
    // ALL BASE HYPER PARAMETERS (NOT EXPANDED)
    _baseHyperParameters = {block1,block2,block3,block4,block5,block6,block7};
    
    
    // BB Output type
    _bbot={ NOMAD::OBJ };
    
    // Max BB eval
    _maxBbEval = 100;
    
    // BB_EXE minus the dataset name (dataset name is added during check
    _bbEXE = "$python ./pytorch_bb.py ";
    
    // Default X0 compatible with the default structure (default values in structure are changed)
    const double x0[]={2 , // Number of convolutionnal layers
        6 , 5 , 1 , 0 , 1 ,
        16 , 5 , 1 , 0 , 1 ,
        2 , // Number of adjsutable full layers
        128,
        84 ,
        _numberOfClasses.value() , // Number of classes taken from the dataset
        128 ,  // Batch size
        3,  0.1 ,0.9 , 0.0005,  0 , // Choice of optimizer + optimizer setting
        0.2, // Dropout rate
        1 // Activation function
    };
    
    size_t dim_x0 = sizeof(x0) / sizeof(double);
    _X0.reset ( static_cast<int>(dim_x0) );
    for ( int i=0 ; i < dim_x0 ; i++ )
        _X0[i]=x0[i];
}



// Expand in block associated parameter by copying multiple time (if type allows) the existing parameters
void HyperParameters::HyperParametersBlock::expandAssociatedParameters()
{
    if ( groupsOfAssociatedHyperParameters.size() > 1 )
    {
        std::string err = "More than one group of associated hyperparameters. The expansion in block " +  name + " has already been done.";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }
    
    if ( associatedParametersType == AssociatedHyperParametersType::ZERO_TIME && groupsOfAssociatedHyperParameters.size() > 0 )
    {
        std::string err = "There is a group of associated hyperparameters for "+name+" but the type is defined as ZERO_TIME.";
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
            std::string err = "The dimension of an hyperparameter block (head parameter " + headOfBlockHyperParameter.fullName + ") is invalid ";
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

// Get an updated group of associated hyperparameters
std::vector<HyperParameters::GenericHyperParameter> HyperParameters::HyperParametersBlock::updateAssociatedParameters ( std::vector<HyperParameters::GenericHyperParameter> & fromGroup ) const
{

    if ( fromGroup.empty() )
    {
        std::string err = "Impossible to update associated parameters (" +  name +")";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }
    
    // By default all attributes are copied ---> case aHP.reportValueType = COPY_VALUE
    std::vector<GenericHyperParameter> copiedGroup(fromGroup);
    
    // UPDATE THE COPIED GROUP ACCORDING TO CONSTRAINTS REPORTING VALUE
    for ( auto & aHP : copiedGroup )
    {
        if ( aHP.reportValueType == ReportValueType::COPY_INITIAL_VALUE && aHP.initialValue.is_defined() )
        {
            aHP.value = aHP.initialValue;
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
                std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = updateAssociatedParameters ( groupsOfAssociatedHyperParameters.back() );
                groupsOfAssociatedHyperParameters.push_back( tmpAssociatedHyperParameters ) ;
            }
        }
        
        // Cases with PLUS ONE at LEFT
        if ( neighborType == NeighborType::PLUS_ONE_MINUS_ONE_LEFT || neighborType == NeighborType::LOOP_PLUS_ONE_LEFT )
        {
            // Expand the associated parameters by copying multiple times before first group
            while ( groupsOfAssociatedHyperParameters.size() < headValue.round() )
            {
                std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = updateAssociatedParameters ( groupsOfAssociatedHyperParameters[0] );
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
            // Reduce the associated parameters by erasing multiple times the last group
            while ( groupsOfAssociatedHyperParameters.size() > headValue.round() )
            {
                groupsOfAssociatedHyperParameters.pop_back( ) ;
                updateAssociatedParameters ( groupsOfAssociatedHyperParameters.back() );
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
                    updateAssociatedParameters ( groupsOfAssociatedHyperParameters[0] );
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

void HyperParameters::HyperParametersBlock::updateAssociatedParameters( NOMAD::Point & x , NOMAD::Point & lb , NOMAD::Point & ub )
{
    
    // update associated parameters from x
    for ( auto & aGroupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : aGroupAHP )
        {
            if ( x.is_defined() )
            {
                // Update the value
                aHP.value = x[0];
                // Trim x from the used value
                trimLeft( x );
            }
            
            // When LOWER_BOUND is used it supersedes other ways of setting bounds (default or by name of hyperparameter)
            if ( lb.is_defined() )
            {
                aHP.lowerBoundValue = lb[0];
                trimLeft( lb );
            }
            // Idem UPPER_BOUND
            if ( ub.is_defined() )
            {
                aHP.upperBoundValue = ub[0];
                trimLeft( ub );
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
            std::string err = "The hyperparameter " + headOfBlockHyperParameter.searchName + " has no value defined.";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        if (  headOfBlockHyperParameter.initialValue.is_defined() )
        {
            std::string err = "The hyperparameter " + headOfBlockHyperParameter.searchName + " already has an init value. Initial value should be set only once";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        
        headOfBlockHyperParameter.initialValue = headOfBlockHyperParameter.value;
        
        if ( headOfBlockHyperParameter.lowerBoundValue.is_defined() && headOfBlockHyperParameter.value < headOfBlockHyperParameter.lowerBoundValue )
        {
            std::string err = "The hyperparameter " + headOfBlockHyperParameter.searchName + " has a lower bound not consistent with the initial value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        if ( headOfBlockHyperParameter.upperBoundValue.is_defined() && headOfBlockHyperParameter.value > headOfBlockHyperParameter.upperBoundValue )
        {
            std::string err = "The hyperparameter " + headOfBlockHyperParameter.searchName + " has an upper bound not consistent with the initial value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
    }
    
    for ( auto & groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : groupAHP )
        {
            if ( aHP.type == NOMAD::CATEGORICAL )
            {
                std::string err = "Only head parameter can be of type CATEGORICAL. Invalid parameter " + aHP.searchName + ".";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
            
            if ( ! aHP.value.is_defined() )
            {
                std::string err = "The hyperparameter " + aHP.searchName + " has no value defined.";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
            
            if (  aHP.initialValue.is_defined() )
            {
                std::string err = "The hyperparameter " + aHP.searchName + " already has an init value. Initial value should be set only once";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
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
            // Put the head hyperparameter value to the lower bound
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
            // Put the head hyperparameter value to the upper bound
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


