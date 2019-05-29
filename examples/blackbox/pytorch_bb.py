# ------------------------------------------------------------------------------
#  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with
#		NOMAD.                                                  
#                                                                              
#                                                   
#                                                                              
#  This program is free software: you can redistribute it and/or modify it     
#  under the terms of the GNU Lesser General Public License as published by    
#  the Free Software Foundation, either version 3 of the License, or (at your  
#  option) any later version.                                                  
#                                                                              
#  This program is distributed in the hope that it will be useful, but WITHOUT 
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
#  for more details.                                                           
#                                                                              
#  You should have received a copy of the GNU Lesser General Public License    
#  along with this program. If not, see <http://www.gnu.org/licenses/>.        
#                                                                              
#  You can find information on the NOMAD software at www.gerad.ca/nomad        
# ------------------------------------------------------------------------------


import os
import sys

if (len(sys.argv) != 3):
    print ('Usage of pytorch_bb.py: DATABASE_NAME X.txt')
    exit()

fin = open(sys.argv[2], 'r')
Lin = fin.readlines()
Xin = Lin[0].split()
fin.close()

syst_cmd = 'OMP_NUM_THREADS=3 python ./blackbox/blackbox.py '+ sys.argv[1] +' '
for i in range(len(Xin)):
    syst_cmd += str(Xin[i]) + ' '

syst_cmd += '> out.txt 2>&1'
os.system(syst_cmd)

# print( syst_cmd )

fout = open('out.txt', 'r')
Lout = fout.readlines()
for line in Lout:
    if "Final accuracy" in line:
        tmp = line.split()
        print('-' + str(tmp[3]))
        fout.close()
        exit()

print('Inf')
fout.close()
