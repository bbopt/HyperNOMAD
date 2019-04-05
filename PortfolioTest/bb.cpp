/*-------------------------------------------------------------------*/
/*            Example of a problem with categorical variables        */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*  . portfolio problem with 3 assets                                */
/*                                                                   */
/*  . the number of variables can be 3,5, or 7, depending on the     */
/*    number of assets considered in the portfolio                   */
/*                                                                   */
/*  . variables are of the form (n t0 v0 t1 v1 t2 v2) where n is the */
/*    number of assets, ti is the type of an asset, and vi is the    */
/*    money invested in this asset                                   */
/*                                                                   */
/*  . categorical variables are n and the ti's                       */
/*                                                                   */
/*  . with a $10,000 budget, the problem consists in minimizing      */
/*    some measure of the risk and of the revenue                    */
/*                                                                   */
/*-------------------------------------------------------------------*/
#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>

int main ( int argc , char ** argv )
{
    double c1,c2,f;
    c1=c2=f=1E20;
    
    std::vector<double> x;
    if ( argc >= 2 )
    {
        std::ifstream in ( argv[1] );
        double v;
        while ( !in.fail() )
        {
            in >> v;
            x.push_back(v);
        }
        
        in.close();
        
        try {
            
            int n = static_cast<int> ( x[0] );
            double vmin = 10000 , tmp;
            double v[3];
            for ( int i = 0 ; i < n ; ++i )
            {
                tmp = v [ static_cast<int> ( x[2*i+1] ) ] = x[2*i+2];
                if ( tmp < vmin )
                    vmin = tmp;
            }
            
            // get the asset types and values:
            
            // constraints (budget and each asset is considered with at least 1$):
            double vt = v[0] + v[1] + v[2];
            c1  = vt - 10000;
            c2 = 1-vmin ;
            
            if ( c1 <= 0 && vmin >= 1 )
            {
                // compute the risk and revenue:
                double vt2  = pow(vt,2);
                double rev  = v[0] * 0.0891 + v[1] * 0.2137 + v[2] * 0.2346;
                double risk = 0.01 * pow(v[0]/vt,2) +
                0.05 * pow (v[1]/vt,2) +
                0.09 * pow (v[2]/vt,2) +
                0.02 * (v[0]*v[1]/vt2)  +
                0.02 * (v[0]*v[2]/vt2)  +
                0.10 * (v[1]*v[2]/vt2);
                
                // the objective is taken as a scaled distance
                // between (risk,revenue) and (risk_best,rev_best):
                double a = ( risk - 0.01 ) * 100 / 0.08;
                double b = ( rev  - 891  ) * 100 / 1455;
                
                f = pow( pow( a,2)+ pow(100-b,2) ,0.5 );
            }
            else
                f=  145 ;
            
            
        }
        catch ( std::exception &)
        {
            return 0;
        }
    }
    std::cout << c1 << " " << c2 << " " << f << std::endl;
    return 0;
    
}

