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


NOMAD::Point HyperParameters::getValues( valueType t) const
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
    if ( hyperParamFileName.empty() )
        initBlockStructureToDefault();
    else
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot initialize with a file. Not yet implemented." );

    if ( _X0.is_defined() )
        update(_X0); // expansion is performed during update
    else
        // Need to expand the HyperParams to have all attributes set
        expand();

    // Perform check on hyper parameters and set initial value for both base and expander
    check();
}

HyperParameters::HyperParameters ( const std::vector<HyperParametersBlock> & hyperParamBlocks )
{

    // The vector of blocks is an expanded structure put into the object
    // This is equivalent to an assignement
    for ( const auto & block : hyperParamBlocks )
        _expandedHyperParameters.push_back( block );
}

void HyperParameters::check( void )
{
    for ( auto & aHyperParameterBlock : _baseHyperParameters )
    {
        aHyperParameterBlock.check();
    }
    for ( auto & aHyperParameterBlock : _expandedHyperParameters )
    {
        aHyperParameterBlock.check();
    }
}

void HyperParameters::initBlockStructureToDefault ( void )
{
    //
    // The default block structure corresponds to MNIST
    //


    // FIRST HYPER PARAMETERS BLOCK (Convolutionnal layers)
    GenericHyperParameter headOfBlock1={"Number of convolutionnal layers",NOMAD::CATEGORICAL,2,0,100};

    GenericHyperParameter hp1={"Number of output channels",NOMAD::INTEGER,6,1,1000,COPY_VALUE};
    GenericHyperParameter hp2={"Kernel size",NOMAD::INTEGER,5,1,20,COPY_VALUE};
    GenericHyperParameter hp3={"Stride",NOMAD::INTEGER,1,1,3,COPY_VALUE};
    GenericHyperParameter hp4={"Padding",NOMAD::INTEGER,0,0,2,COPY_VALUE};
    GenericHyperParameter hp5={"Do a pooling",NOMAD::BINARY,0,0,1,COPY_VALUE};
    GroupsOfAssociatedHyperParameters associatedHyperParameters1={{hp1,hp2,hp3,hp4,hp5}};

    HyperParametersBlock block1={"Convolutionnal layers",headOfBlock1,PLUS_ONE_MINUS_ONE_RIGHT,MULTIPLE_TIMES,associatedHyperParameters1};


    // SECOND CATEGORICAL BLOCK (Full layers)
    GenericHyperParameter headOfBlock2={"Number of full layers",NOMAD::CATEGORICAL,3,0,500};

    GenericHyperParameter hp6={"Size of a full layer",NOMAD::INTEGER,128,1,1000,COPY_VALUE,FIXED_IF_IN_LAST_GROUP,NOMAD::Double(10.0) };
    GroupsOfAssociatedHyperParameters associatedHyperParameters2={{hp6}};

    HyperParametersBlock block2={"Full  layers",headOfBlock2,PLUS_ONE_MINUS_ONE_LEFT,MULTIPLE_TIMES,associatedHyperParameters2};

    // THIRD BLOCK (single regular parameter: batch size)
    GenericHyperParameter headOfBlock3={"Batch size",NOMAD::INTEGER,128,1,400,NO_REPORT,NEVER_FIXED};
    HyperParametersBlock block3={"Batch size",headOfBlock3,NONE,ZERO_TIME,};

    // FOURTH CATEGORICAL BLOCK (Optimizer select)
    GenericHyperParameter headOfBlock4={"Choice of optimizer",NOMAD::CATEGORICAL,3,1,4};

    GenericHyperParameter hp7={"Learning rate",NOMAD::CONTINUOUS,0.1,0,1,COPY_INITIAL_VALUE};
    GenericHyperParameter hp8={"Momentum",NOMAD::CONTINUOUS,0.9,0,1,COPY_INITIAL_VALUE};
    GenericHyperParameter hp9={"Weight decay",NOMAD::CONTINUOUS,0.0005,0,1,COPY_INITIAL_VALUE};
    GenericHyperParameter hp10={"Dampening",NOMAD::CONTINUOUS,0,0,1,COPY_INITIAL_VALUE};
    GroupsOfAssociatedHyperParameters associatedHyperParameters4={{hp7,hp8,hp9,hp10}};

    HyperParametersBlock block4={"Optimizer",headOfBlock4,LOOP_PLUS_ONE_RIGHT,ONE_TIME,associatedHyperParameters4};

    // FIFTH BLOCK (single regular parameter: Dropout rate)
    GenericHyperParameter headOfBlock5={"Dropout rate",NOMAD::CONTINUOUS,0.2,0,0.75};
    HyperParametersBlock block5={"Dropout rate",headOfBlock5,NONE,ZERO_TIME,};

    // SIXTH BLOCK (single regular parameter: Activation function)
    GenericHyperParameter headOfBlock6={"Activation function",NOMAD::INTEGER,1,1,3};
    HyperParametersBlock block6={"Activation function",headOfBlock6,NONE,ZERO_TIME,};

    // ALL BASE HYPER PARAMETERS (NOT EXPANDED)
    _baseHyperParameters = {block1,block2,block3,block4,block5,block6};


    // Database name
    _databaseName = " ";

    // BB
    // _bbEXE = "$python ./pytorch_bb.py";
    _bbEXE = "$perl ./bbFromHistory.pl";
    
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


//void HyperParameters::initBlockStructureToDefault ( void )
//{
//    //
//    // The default block structure corresponds to CIPHAR10
//    //
//
//
//    // FIRST HYPER PARAMETERS BLOCK (Convolutionnal layers)
//    GenericHyperParameter headOfBlock1={"Number of convolutionnal layers",NOMAD::CATEGORICAL,6,0,100};
//
//    GenericHyperParameter hp1={"Number of output channels",NOMAD::INTEGER,16,1,1000,COPY_VALUE};
//    GenericHyperParameter hp2={"Kernel size",NOMAD::INTEGER,3,1,20,COPY_VALUE};
//    GenericHyperParameter hp3={"Stride",NOMAD::INTEGER,1,1,3,COPY_VALUE};
//    GenericHyperParameter hp4={"Padding",NOMAD::INTEGER,0,0,2,COPY_VALUE};
//    GenericHyperParameter hp5={"Do a pooling",NOMAD::BINARY,0,0,1,COPY_VALUE};
//    GroupsOfAssociatedHyperParameters associatedHyperParameters1={{hp1,hp2,hp3,hp4,hp5}};
//
//    HyperParametersBlock block1={"Convolutionnal layers",headOfBlock1,PLUS_ONE_MINUS_ONE_RIGHT,MULTIPLE_TIMES,associatedHyperParameters1};
//
//
//    // SECOND CATEGORICAL BLOCK (Full layers)
//    GenericHyperParameter headOfBlock2={"Number of full layers",NOMAD::CATEGORICAL,2,0,30};
//
//    GenericHyperParameter hp6={"Size of a full layer",NOMAD::INTEGER,100,0,500,COPY_VALUE,FIXED_IF_IN_LAST_GROUP,NOMAD::Double(10.0) };
//    GroupsOfAssociatedHyperParameters associatedHyperParameters2={{hp6}};
//
//    HyperParametersBlock block2={"Full  layers",headOfBlock2,PLUS_ONE_MINUS_ONE_LEFT,MULTIPLE_TIMES,associatedHyperParameters2};
//
//    // THIRD BLOCK (single regular parameter: batch size)
//    GenericHyperParameter headOfBlock3={"Batch size",NOMAD::INTEGER,64,1,500,NO_REPORT,NEVER_FIXED};
//    HyperParametersBlock block3={"Batch size",headOfBlock3,NONE,ZERO_TIME,};
//
//    // FOURTH CATEGORICAL BLOCK (Optimizer select)
//    GenericHyperParameter headOfBlock4={"Choice of optimizer",NOMAD::CATEGORICAL,0,0,3};
//
//    GenericHyperParameter hp7={"Learning rate",NOMAD::CONTINUOUS,0.001,0,1,COPY_INITIAL_VALUE};
//    GenericHyperParameter hp8={"Momentum | Beta1 | Learning rate decay ",NOMAD::CONTINUOUS,0.9,0,1,COPY_INITIAL_VALUE};
//    GenericHyperParameter hp9={"Dampening | Beta2 | Initial accumulator | alpha ",NOMAD::CONTINUOUS,0.99,0,1,COPY_INITIAL_VALUE};
//    GenericHyperParameter hp10={"Weight decay ",NOMAD::CONTINUOUS,0,0,1,COPY_INITIAL_VALUE};
//    GroupsOfAssociatedHyperParameters associatedHyperParameters4={{hp7,hp8,hp9,hp10}};
//
//    HyperParametersBlock block4={"Optimizer",headOfBlock4,LOOP_PLUS_ONE_RIGHT,ONE_TIME,associatedHyperParameters4};
//
//    // FIFTH BLOCK (single regular parameter: Dropout rate)
//    GenericHyperParameter headOfBlock5={"Dropout rate",NOMAD::CONTINUOUS,0.5,0,1};
//    HyperParametersBlock block5={"Dropout rate",headOfBlock5,NONE,ZERO_TIME,};
//
//    // SIXTH BLOCK (single regular parameter: Activation function)
//    GenericHyperParameter headOfBlock6={"Activation function",NOMAD::INTEGER,1,0,2};
//    HyperParametersBlock block6={"Activation function",headOfBlock6,NONE,ZERO_TIME,};
//
//    // ALL BASE HYPER PARAMETERS (NOT EXPANDED)
//    _baseHyperParameters = {block1,block2,block3,block4,block5,block6};
//
//
//    // Database name
//    _databaseName = " ";
//
//    // BB
//    _bbEXE = "$python ./pytorch_bb.py";
//
//    // BB Output type
//    _bbot={ NOMAD::OBJ };
//
//    // Max BB eval
//    _maxBbEval = 100;
//
//    const double x0[]={6, 16, 3, 1, 0, 0, 32, 3, 2, 0, 0, 32, 3, 1, 0, 0, 64, 3, 1, 0, 0, 64, 3, 1, 0, 0, 128, 3, 2, 0, 0, 2, 100, 10, 64, 2, 0.001, 0.9, 0.999, 0, 0, 1};
//    size_t dim_x0 = sizeof(x0) / sizeof(double);
//    _X0.reset ( static_cast<int>(dim_x0) );
//    for ( int i=0 ; i < dim_x0 ; i++ )
//        _X0[i]=x0[i];
//
//}


// Expand in block associated parameter by copying multiple time (if type allows) the existing parameters
void HyperParameters::HyperParametersBlock::expandAssociatedParameters()
{
    if ( groupsOfAssociatedHyperParameters.size() > 1 )
    {
        std::string err = "More than one group of associated hyper parameters. The expansion in block " +  name + " has already been done.";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( associatedParametersType == ZERO_TIME && groupsOfAssociatedHyperParameters.size() > 0 )
    {
        std::string err = "There is a group of associated hyper parameters for "+name+" but the type is defined as ZERO_TIME.";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( headOfBlockHyperParameter.type != NOMAD::CATEGORICAL && neighboorType != NONE )
    {
        std::string err = "Only categorical variables can have a neighboor type different than NONE. Head parameter " + headOfBlockHyperParameter.name + " is invalid ";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && neighboorType == NONE )
    {
        std::string err = "Categorical variables must have a neighboor type different than NONE. Head parameter " + headOfBlockHyperParameter.name + " is invalid ";
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    // The expansion is performed only for MULTIPLE_TIMES associated parameters
    if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && associatedParametersType == MULTIPLE_TIMES ) // The head parameter of block has associated parameters
    {
        NOMAD::Double headValue = headOfBlockHyperParameter.value;

        if ( ! headValue.is_defined() || ! headValue.is_integer() || headValue < 0 )
        {
            std::string err = "The dimension of an hyper parameter block (head parameter " + headOfBlockHyperParameter.name + ") is invalid ";
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
    if ( headValue.is_defined() && associatedParametersType == MULTIPLE_TIMES && groupsOfAssociatedHyperParameters.size() != headValue.round() )
    {
        std::string err = "The number of groups of associated parameters is inconsistent with the head value for " + name ;
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
    }

    for ( auto & aGroupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : aGroupAHP )
        {
            // First group only
            if ( aHP.fixedParamType == FIXED_IF_IN_FIRST_GROUP && &aGroupAHP == &groupsOfAssociatedHyperParameters.front() )
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
            if ( aHP.fixedParamType == FIXED_IF_IN_LAST_GROUP && &aGroupAHP == &groupsOfAssociatedHyperParameters.back() )
            {
                aHP.isFixed = true;
                if ( ! aHP.fixedValue.is_defined() )
                {
                    std::string err = "Cannot fix associated parameter " + name +" when in first group because the fixedValue is not provided";
                    throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
                }
                aHP.value = aHP.fixedValue;
            }

            if ( aHP.fixedParamType == ALWAYS_FIXED )
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
        if ( aHP.fixedParamType == ALWAYS_FIXED && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = true;
            aHP.value = aHP.fixedValue;
            continue;
        }
        if ( aHP.fixedParamType == FIXED_IF_IN_LAST_GROUP && isLastGroup && aHP.fixedValue.is_defined() )
        {
            aHP.value = aHP.fixedValue;
            aHP.isFixed = true ;
            continue;
        }
        if ( aHP.fixedParamType == FIXED_IF_IN_FIRST_GROUP && isFirstGroup && aHP.fixedValue.is_defined() )
        {
            aHP.value = aHP.fixedValue;
            aHP.isFixed = true ;
            continue;
        }
        if ( aHP.reportValueType == COPY_INITIAL_VALUE && aHP.initialValue.is_defined() )
        {
            aHP.value = aHP.initialValue;
            continue;
        }
    }

    // UPDATE THE FROM GROUP ACCORDING TO CONSTRAINTS FOR FIXING
    for ( auto & aHP : fromGroup )
    {
        if ( aHP.fixedParamType == ALWAYS_FIXED && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = true;
            aHP.value = aHP.fixedValue;
            continue;
        }
        // The fromGroup is not the last anymore --> remove the fixed flag
        if ( aHP.fixedParamType == FIXED_IF_IN_LAST_GROUP && isLastGroup && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = false ;
            continue;
        }
        if ( aHP.fixedParamType == FIXED_IF_IN_FIRST_GROUP && isFirstGroup && aHP.fixedValue.is_defined() )
        {
            aHP.isFixed = false;
            continue;
        }
    }



    return copiedGroup;
}


void HyperParameters::HyperParametersBlock::expandAndUpdateAssociatedParametersWithConstraints ( void )
{
    if ( neighboorType == NONE )
        return;

    if ( associatedParametersType == MULTIPLE_TIMES ) // The head parameter of block has associated parameters
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
        if ( neighboorType == PLUS_ONE_MINUS_ONE_RIGHT || neighboorType == LOOP_PLUS_ONE_RIGHT )
        {
            // Expand the associated parameters by copying multiple times at the end of first group
            while ( groupsOfAssociatedHyperParameters.size() < headValue.round() )
            {
                std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = updateAssociatedParameters ( groupsOfAssociatedHyperParameters.back() , true , false );
                groupsOfAssociatedHyperParameters.push_back( tmpAssociatedHyperParameters ) ;
            }
        }

        // Cases with PLUS ONE at LEFT
        if ( neighboorType == PLUS_ONE_MINUS_ONE_LEFT || neighboorType == LOOP_PLUS_ONE_LEFT )
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
    if ( neighboorType == NONE )
        return;

    if ( associatedParametersType == MULTIPLE_TIMES ) // The head parameter of block has associated parameters
    {
        //
        // Reduce the groups of associated parameters to head value without considering the lower bound
        //


        NOMAD::Double headValue = headOfBlockHyperParameter.value;
        NOMAD::Double headLowerBound = headOfBlockHyperParameter.lowerBoundValue;

        if ( ! headValue.is_defined() || headValue < headLowerBound )
            return;


        // Cases with MINUS ONE at RIGHT
        if ( neighboorType == PLUS_ONE_MINUS_ONE_RIGHT || neighboorType == LOOP_MINUS_ONE_RIGHT )
        {
            // Reduce the associated parameters by erasing multiple times at the last group
            while ( groupsOfAssociatedHyperParameters.size() > headValue.round() )
            {
                groupsOfAssociatedHyperParameters.pop_back( ) ;
                updateAssociatedParameters ( groupsOfAssociatedHyperParameters.back() , true , false );
            }
        }

        // Cases with MINUS ONE at LEFT
        if ( neighboorType == PLUS_ONE_MINUS_ONE_LEFT || neighboorType == LOOP_MINUS_ONE_LEFT )
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
                std::string err = "Only head parameter can be of type CATEGORICAL. Invalid parameter " + aHP.name + ".";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            // Update the value
            aHP.value = x[0];

            // Trim x from the used value
            trimLeft( x );

            if ( aHP.isFixed && aHP.value != aHP.fixedValue )
            {
                std::string err = "Cannot set the fixed variable " + aHP.name + " to a value different than the provided fixedValue.";
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
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.name + " has no value defined.";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        if (  headOfBlockHyperParameter.initialValue.is_defined() )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.name + " already has an init value. Initial value should be set only once";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }

        if (  headOfBlockHyperParameter.fixedParamType == ALWAYS_FIXED && headOfBlockHyperParameter.fixedValue.is_defined() && headOfBlockHyperParameter.value != headOfBlockHyperParameter.fixedValue )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.name + " is always fixed but has an init value not consistent with the fixed value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        headOfBlockHyperParameter.initialValue = headOfBlockHyperParameter.value;

        if ( headOfBlockHyperParameter.lowerBoundValue.is_defined() && headOfBlockHyperParameter.value < headOfBlockHyperParameter.lowerBoundValue )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.name + " has a lower bound not consistent with the initial value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        if ( headOfBlockHyperParameter.upperBoundValue.is_defined() && headOfBlockHyperParameter.value > headOfBlockHyperParameter.upperBoundValue )
        {
            std::string err = "The hyper parameter " + headOfBlockHyperParameter.name + " has an upper bound not consistent with the initial value";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
    }

    for ( auto & groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto & aHP : groupAHP )
        {
            if ( ! aHP.value.is_defined() )
            {
                std::string err = "The hyper parameter " + aHP.name + " has no value defined.";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if (  aHP.initialValue.is_defined() )
            {
                std::string err = "The hyper parameter " + aHP.name + " already has an init value. Initial value should be set only once";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.fixedParamType == ALWAYS_FIXED && aHP.fixedValue.is_defined() && aHP.value != aHP.fixedValue )
            {
                std::string err = "The hyper parameter " + aHP.name + " has an init value not consistent with the fixed value";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
            if ( aHP.isFixed && ! aHP.fixedValue.is_defined() )
            {
                std::string err = "The hyper parameter " + aHP.name + " is fixed but has no fixed value";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.isFixed && aHP.fixedParamType == NEVER_FIXED )
            {
                std::string err = "The hyper parameter " + aHP.name + " is fixed but has no fixed value";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }

            if ( aHP.fixedParamType == ALWAYS_FIXED )
            {
                aHP.isFixed = true;
                aHP.fixedValue = aHP.value;
            }

            if ( aHP.isFixed )
            {
                aHP.fixedParamType = ALWAYS_FIXED;
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

std::vector<NOMAD::Double> HyperParameters::HyperParametersBlock::getAssociatedValues ( valueType t ) const
{
    std::vector<NOMAD::Double> values;
    for ( auto groupAHP : groupsOfAssociatedHyperParameters )
    {
        for ( auto aHP : groupAHP )
        {
            if ( t == CURRENT_VALUE )
                values.push_back( aHP.value );
            else if ( t == LOWER_BOUND )
                values.push_back( aHP.lowerBoundValue );
            else if ( t == UPPER_BOUND )
                values.push_back ( aHP.upperBoundValue );
            else if ( t == INITIAL_VALUE )
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

std::vector<NOMAD::Double> HyperParameters::HyperParametersBlock::getValues( valueType t ) const
{

    std::vector<NOMAD::Double> values;

    if ( headOfBlockHyperParameter.isDefined() )
    {
        if ( t == CURRENT_VALUE )
            values.push_back( headOfBlockHyperParameter.value  );
        else if ( t == LOWER_BOUND )
        {
            // If head is Categorical, the provided lower bound is for limiting the possible neighboors of a point. A categorical variable does not need bounds for optimization
            if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL )
                values.push_back( NOMAD::Double() );
            else
                values.push_back( headOfBlockHyperParameter.lowerBoundValue  );
        }
        else if ( t == UPPER_BOUND )
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
    if ( neighboorType == NONE || headOfBlockHyperParameter.type != NOMAD::CATEGORICAL )
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
    if ( neighboorType == PLUS_ONE_MINUS_ONE_RIGHT || neighboorType == PLUS_ONE_MINUS_ONE_LEFT )
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

    if ( neighboorType == LOOP_PLUS_ONE_LEFT
        || neighboorType == LOOP_PLUS_ONE_RIGHT )
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

    if ( neighboorType == LOOP_MINUS_ONE_LEFT
        || neighboorType == LOOP_MINUS_ONE_RIGHT )
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
