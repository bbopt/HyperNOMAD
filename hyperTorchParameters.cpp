//
//  hyperTorchParameters.cpp
//  HyperNomad
//
//  Created by Christophe Tribes on 19-03-28.
//  Copyright © 2019 GERAD. All rights reserved.
//

#include "hyperTorchParameters.hpp"

void HyperTorchParameters::read ( const std::string & hyperParamFileName )
{
    return;
    
}
void HyperTorchParameters::init ( void )
{
    
// FIRST HYPER PARAMETERS BLOCK (Convolutionnal layers)
    GenericHyperParameter headOfBlock1={"Number of convolutionnal layers",NOMAD::CATEGORICAL,5,0,20,NO_REPORT};
    
    GenericHyperParameter hp1={"Number of output channels",NOMAD::INTEGER,5,0,50,COPY_VALUE};
    GenericHyperParameter hp2={"Kernel size",NOMAD::INTEGER,5,0,10,COPY_VALUE};
    GenericHyperParameter hp3={"Stride",NOMAD::INTEGER,2,1,3,COPY_VALUE};
    GenericHyperParameter hp4={"Padding",NOMAD::INTEGER,1,0,2,COPY_VALUE};
    GenericHyperParameter hp5={"Do a pooling",NOMAD::BINARY,0,0,1,COPY_VALUE};
    std::vector<GenericHyperParameter> inBlockHP1={hp1,hp2,hp3,hp4,hp5};

    HyperParametersBlock block1={"Convolutionnal layers",headOfBlock1,PLUS_ONE_MINUS_ONE,MULTIPLE_TIMES,inBlockHP1};

    
// SECOND CATEGORICAL BLOCK (Full layers)
    GenericHyperParameter headOfBlock2={"Number of full layers",NOMAD::CATEGORICAL,15,0,30,NO_REPORT};
    
    GenericHyperParameter hp6={"Size of a full layer",NOMAD::INTEGER,100,0,500,COPY_VALUE};
    std::vector<GenericHyperParameter> inBlockHP2={hp6};
    
    HyperParametersBlock block2={"Full layers layers",headOfBlock2,PLUS_ONE_MINUS_ONE,MULTIPLE_TIMES,inBlockHP2};

// THIRD BLOCK (single regular parameter: Dropout rate)
    GenericHyperParameter headOfBlock3={"Dropout rate",NOMAD::CONTINUOUS,0.5,0,1,NO_REPORT};    
    HyperParametersBlock block3={"Dropout rate",headOfBlock3,NONE,ZERO_TIME,};
    
// FOURTH CATEGORICAL BLOCK (single categorical parameter: Activation function)
    GenericHyperParameter headOfBlock4={"Activation function",NOMAD::CATEGORICAL,1,0,2,NO_REPORT};
    HyperParametersBlock block4={"Dropout rate",headOfBlock3,LOOP_PLUS_ONE,ZERO_TIME,};
    
// FITH CATEGORICAL BLOCK (Optimizer select)
    GenericHyperParameter headOfBlock5={"Choice of optimizer",NOMAD::CATEGORICAL,0,0,3,NO_REPORT};
    
    GenericHyperParameter hp7={"Learning rate",NOMAD::CONTINUOUS,0.5,0,1,INITIAL_VALUE};
    GenericHyperParameter hp8={"Momentum | Beta1 | Learning rate decay ",NOMAD::CONTINUOUS,0.5,0,1,INITIAL_VALUE};
    GenericHyperParameter hp9={"Dampening | Beta2 | Initial accumulator | alpha ",NOMAD::CONTINUOUS,0.5,0,1,INITIAL_VALUE};
    GenericHyperParameter hp10={"Weight decay ",NOMAD::CONTINUOUS,0.5,0,1,INITIAL_VALUE};
    std::vector<GenericHyperParameter> inBlock5={hp7,hp8,hp9,hp10};
    
    HyperParametersBlock block5={"Optimizer",headOfBlock5,LOOP_PLUS_ONE,ONE_TIME,inBlock5};
    
    
// ALL HYPER PARAMETERS
    _allParameters = {block1,block2,block3,block4,block5};
    

// Database name
    _databaseName = " ";
    
// BB
    _bbEXE = "$python ./pytorch_bb.py";
}
