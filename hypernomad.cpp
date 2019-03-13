/* ------------------------------------------------------------------------------	*/
/*  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with		*/
/*		NOMAD.                                                  		*/
/*                                                                              	*/
/*                                                   					*/
/*                                                                              	*/
/*  This program is free software: you can redistribute it and/or modify it     	*/
/*  under the terms of the GNU Lesser General Public License as published by    	*/
/*  the Free Software Foundation, either version 3 of the License, or (at your  	*/
/*  option) any later version.                                                  	*/
/*                                                                              	*/
/*  This program is distributed in the hope that it will be useful, but WITHOUT 	*/
/*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       	*/
/*  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 	*/
/*  for more details.                                                           	*/
/*                                                                              	*/
/*  You should have received a copy of the GNU Lesser General Public License    	*/
/*  along with this program. If not, see <http://www.gnu.org/licenses/>.        	*/
/*                                                                              	*/
/*  You can find information on the NOMAD software at www.gerad.ca/nomad        	*/
/* ------------------------------------------------------------------------------	*/



/*-------------------------------------------------------------------*/
/*            Example of a problem with categorical variables        */
/*-------------------------------------------------------------------*/
#include "nomad.hpp"
#include <vector>

using namespace std;
using namespace NOMAD;

#define USE_SURROGATE false
/*--------------------------------------------------*/
/*  user class to define categorical neighborhoods  */
/*--------------------------------------------------*/
class My_Extended_Poll : public Extended_Poll
{
	
private:
	
	// vector of signatures
        int _extended_poll_call;
        Signature * _s0 ,* _s1 , * _s2 , * _s3 , * _s4 ;
        size_t  _dim_0 , _dim1, _dim_2, _dim_3, _dim_4 ;
	
public:
	
	// constructor:
	My_Extended_Poll ( Parameters & );
	
	// destructor:
	virtual ~My_Extended_Poll ( void ) {
            delete _s0;
            delete _s1;
            delete _s2;
            delete _s3;
            delete _s4;
            }
	
	// construct the extended poll points:
	virtual void construct_extended_points ( const Eval_Point &);
	
}; 


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main ( int argc , char ** argv )
{
	
	// NOMAD initializations:
	begin ( argc , argv );
	
	// display:
	Display out ( cout );
	out.precision ( DISPLAY_PRECISION_STD );
	
	try
	{
		
		// parameters creation:
		Parameters p ( out );
		
		if ( USE_SURROGATE )
			p.set_HAS_SGTE ( true );

                
                p.read("parameter_file.txt");
                
                p.set_EXTENDED_POLL_TRIGGER ( 10 , false );
		
		// parameters validation:
		p.check();
				
		// extended poll:
		My_Extended_Poll ep ( p );
		
		// algorithm creation and execution:
		Mads mads ( p , NULL , &ep , NULL , NULL );
                mads.run();
	}
	catch ( exception & e ) {
		string error = string ( "NOMAD has been interrupted: " ) + e.what();
		if ( Slave::is_master() )
			cerr << endl << error << endl << endl;
	}
	
	
	Slave::stop_slaves ( out );
	end();
	
	return EXIT_SUCCESS;
}

/*-----------------------------------------*/
/*  constructor: creates the 9 signatures  */
/*-----------------------------------------*/

My_Extended_Poll::My_Extended_Poll ( Parameters & p )
: Extended_Poll ( p    ),
  _s0 (NULL),
  _s1 (NULL),
  _s2 (NULL),
  _s3 (NULL),
  _s4 (NULL)
{
        

}

/*--------------------------------------*/
/*  construct the extended poll points  */
/*      (categorical neighborhoods)     */
/*--------------------------------------*/
void My_Extended_Poll::construct_extended_points ( const Eval_Point & x)
{    

	// shift1 and shift2 correspond to the number of HPs for each 
	// conv layer and each fully connected layer
        int shift1 = 5;
        int shift2 = 1;
        
        // number of convolutional layers
	int n = static_cast<int> (x[0].value());
        
        // number of fully connected layer
        int index_m = shift1*n + 1;
        int m = static_cast<int> (x[index_m].value());
        
	// get the size and type of the current point
        const vector<bb_input_type> & bbit_curr = x.get_signature()->get_input_types();
        int current_dim = bbit_curr.size();
        
        // get the bounds and and directions of the current point
        const Point & d0_curr = x.get_signature()->get_mesh()->get_initial_poll_size();
        const Point & lb_curr = x.get_signature()->get_lb();
        const Point & ub_curr = x.get_signature()->get_ub();
             
	// dimensions of the neighbors      
        int dim1 = current_dim + shift1;
        int dim2 = current_dim - shift1;
        int dim3 = current_dim + shift2;
        int dim4 = current_dim - shift2;
        
        if (n < ub_curr[0]){
            // We can add a convolutional layer
            Point d0_1 (dim1);
            Point lb_1 (dim1);
            Point ub_1 (dim1);

            vector<bb_input_type> bbit_1 (dim1);
            bbit_1[0] = bbit_1[index_m + shift1] = CATEGORICAL;

            for ( int i = 1 ; i < index_m + shift1 ; i++ )
                bbit_1[i] = INTEGER;
        
            for ( int i = index_m + shift1 + 1 ; i < dim1 ; i++ )
                bbit_1[i] = bbit_curr[i-shift1];
 
            // categorical variables don't need bounds
            d0_1[0] = d0_1[index_m + shift1] = 1;

            for (int i = 1; i< index_m; i++){
                lb_1  [i] = lb_curr[i];
                ub_1  [i] = ub_curr[i];
                d0_1  [i] = d0_curr[i];
            }
            
            for ( int i = index_m ; i < index_m + shift1 ; i++ )
            {
                lb_1  [i] = lb_curr[i-shift1];
                ub_1  [i] = ub_curr[i-shift1];
                d0_1  [i] = d0_curr[i-shift1];
            }
            
            for ( int i = index_m + shift1 + 1 ; i < dim1 ; i++ ){
                lb_1  [i] = lb_curr[i-shift1];
                ub_1  [i] = ub_curr[i-shift1];
                d0_1  [i] = d0_curr[i-shift1];
            }
            
            _s1 = new NOMAD::Signature(dim1,
                                bbit_1                     ,
                                d0_1                       ,
                                lb_1                       ,
                                ub_1                       ,
                                _p.get_direction_types   () ,
                                _p.get_sec_poll_dir_types() ,
                                _p.get_int_poll_dir_types() ,
                                _p.out()                    );
            
            
            
            Point y (dim1);
            y[0] = n + 1;
            y[1] = 10;
            y[2] = 5;
            y[3] = 1;
            y[4] = 0;
            y[5] = 0;
            for (int i = 5 ; i < dim1 ; i++)
            {
                y[i] = x[i-shift1];
            }
            add_extended_poll_point (y , *_s1);
	    // cerr << endl << " Done with 1st neighbor." << endl << endl;
        }
        
        if (n>lb_curr[0]){
            // We can substract a convolutional layer
            
            Point d0_2 (dim2);
            Point lb_2 (dim2);
            Point ub_2 (dim2);
        
            vector<bb_input_type> bbit_2 (dim2);
            bbit_2[0] = bbit_2[index_m - shift1] = CATEGORICAL;

            for ( int i = 1 ; i < index_m - shift1 ; i++ )
                bbit_2[i] = INTEGER;
        
            for ( int i = index_m - shift1 ; i < dim2 ; i++ )
                bbit_2[i] = bbit_curr[i+shift1];
 
            // Categorical variables don't need bounds
            d0_2[0] = d0_2[index_m - shift1] = 1;

            for ( int i = 1 ; i < index_m - shift1 ; i++ )
            {
                lb_2  [i] = lb_curr[i+shift1];
                ub_2  [i] = ub_curr[i+shift1];
                d0_2  [i] = d0_curr[i+shift1];
            }
            lb_2  [index_m - shift1] = lb_curr[index_m];
            ub_2  [index_m - shift1] = ub_curr[index_m];
            
            for ( int i = index_m - shift1 +1 ; i < dim2 ; i++ )
            {
                lb_2  [i] = lb_curr[i+shift1];
                ub_2  [i] = ub_curr[i+shift1];
                d0_2  [i] = d0_curr[i+shift1];
            }
            
            _s2 = new NOMAD::Signature(dim2,
                                bbit_2                     ,
                                d0_2                       ,
                                lb_2                       ,
                                ub_2                       ,
                                _p.get_direction_types   () ,
                                _p.get_sec_poll_dir_types() ,
                                _p.get_int_poll_dir_types() ,
                                _p.out()                    );

            
            Point y2 (dim2);
            y2[0] = n - 1;
            // y2[1] = x[1];
            for (int i = 1 ; i < dim2 ; i++)
            {
                y2[i] = x[i+shift1];
            }
            add_extended_poll_point (y2 , *_s2);
            // cerr << endl << " Done with 2nd neighbor." << endl << endl;
                     
        }
        
        if(m<ub_curr[index_m]){
            // We can add a full layer
            Point d0_3 (dim3);
            Point lb_3 (dim3);
            Point ub_3 (dim3);
            
            vector<bb_input_type> bbit_3 (dim3);
            bbit_3[0] = bbit_3[index_m] = CATEGORICAL;

            for ( int i = 1 ; i < index_m ; i++ )
                bbit_3[i] = INTEGER;
            
            bbit_3[index_m + 1] = INTEGER;
            
            for ( int i = index_m + shift2 +1 ; i < dim3 ; i++ )
                bbit_3[i] = bbit_curr[i-shift2];
            
            // Categorical variables don't need bounds
            d0_3[0] = 1;
            
            for ( int i = 1 ; i < index_m ; i++ )
            {
                lb_3  [i] = lb_curr[i];
                ub_3  [i] = ub_curr[i];
                d0_3  [i] = d0_curr[i];
            }
            // Categorical variables don't need bounds
            d0_3[index_m] = 1;
            
            lb_3  [index_m + 1] = lb_curr[index_m + 1];
            ub_3  [index_m + 1] = ub_curr[index_m + 1];
            d0_3  [index_m + 1] = d0_curr[index_m + 1];
            
            for ( int i = index_m + shift2 + 1 ; i < dim3 ; i++ ){
                lb_3  [i] = lb_curr[i-shift2];
                ub_3  [i] = ub_curr[i-shift2];
                d0_3  [i] = d0_curr[i-shift2];
            }
            
            _s3 = new NOMAD::Signature(dim3,
                                bbit_3                     ,
                                d0_3                       ,
                                lb_3                       ,
                                ub_3                       ,
                                _p.get_direction_types   () ,
                                _p.get_sec_poll_dir_types() ,
                                _p.get_int_poll_dir_types() ,
                                _p.out()                    );
            Point y3 (dim3);
            
            for (int i = 0 ; i < index_m ; i++)
            {
                y3[i] = x[i];
            }
            y3[index_m] = m+1;
            y3[index_m + shift2] = 100;
            for (int i = index_m + shift2 + 1 ; i < dim3 ; i++)
            {
                y3[i] = x[i-shift2];
            }
            add_extended_poll_point (y3 , *_s3);
            // cerr << endl << " Done with 3th neighbor." << endl << endl;           
        }
        
        if(m>lb_curr[index_m]){
            // we can substract a layer
            Point d0_4 (dim4);
            Point lb_4 (dim4);
            Point ub_4 (dim4);
            
            vector<bb_input_type> bbit_4 (dim4);
            bbit_4[0] = bbit_4[index_m] = CATEGORICAL;

            for ( int i = 1 ; i < index_m ; i++ )
                bbit_4[i] = INTEGER;
        
            for ( int i = index_m + 1 ; i < dim4 ; i++ )
                bbit_4[i] = bbit_curr[i+shift2];
            
            // Categorical variables don't need bounds
            d0_4[0] = 1;
            
            for ( int i = 1 ; i < index_m ; i++ )
            {
                lb_4  [i] = lb_curr[i];
                ub_4  [i] = ub_curr[i];
                d0_4  [i] = d0_curr[i];
            }
            // Categorical variables don't need bounds
            d0_4[index_m] = 1;
            
            for ( int i = index_m + 1 ; i < dim4 ; i++ ){
                lb_4  [i] = lb_curr[i+shift2];
                ub_4  [i] = ub_curr[i+shift2];
                d0_4  [i] = d0_curr[i+shift2];
            }
            
            _s4 = new NOMAD::Signature(dim4,
                                bbit_4                     ,
                                d0_4                       ,
                                lb_4                       ,
                                ub_4                       ,
                                _p.get_direction_types   () ,
                                _p.get_sec_poll_dir_types() ,
                                _p.get_int_poll_dir_types() ,
                                _p.out()                    );
            Point y4 (dim4);
            
            for (int i = 0 ; i < index_m ; i++)
            {
                y4[i] = x[i];
            }
            y4[index_m] = m-1;
            for (int i = index_m + 1 ; i < dim4 ; i++)
            {
                y4[i] = x[i+shift2];
            }
            add_extended_poll_point (y4 , *_s4);
            // cerr << endl << " Done with 4th neighbor." << endl << endl;
        }
}

