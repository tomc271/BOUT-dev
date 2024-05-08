#!/usr/bin/env bash
        
MPIEXEC=mpirun
NP=2

#-compile/build local executable
make

#-run the case       
echo Running with NP = $NP       

rm -rf data*

zlist=( 1 2 4 8 16 32 64 128 256 )

for zval in ${zlist[@]}
do
  mkdir data_${zval}
  ln -s data_${zval} data
  sed "s/Zeff = 128.0/Zeff = ${zval}.0/g" BOUT.inp > data/BOUT.inp

  if [ $zval -lt 128 ]
      then
      # reduce time-step. At large times these cases produce noise
      sed "s/TIMESTEP = 5e3/TIMESTEP = 1e3/g" data/BOUT.inp > data/tmp
      mv data/tmp data/BOUT.inp
  fi
      
  $MPIEXEC -np $NP ./2fluid
  rm -f data
done

#-check the result
#idl runidl.pro
