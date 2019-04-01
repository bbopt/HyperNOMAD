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

#ifndef hyperTorchParameters_hpp
#define hyperTorchParameters_hpp

#include <stdio.h>
#include "hyperParameters.hpp"

class HyperTorchParameters : public HyperParameters {
    
public:
    HyperTorchParameters(){ init(); expand(); }
    
    virtual void read ( const std::string & hyperParamFileName ) override ;
    virtual void init ( void ) override ;
};

#endif /* hyperTorchParameters_hpp */
