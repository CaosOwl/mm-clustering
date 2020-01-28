#source root
source /opt/root/root-6-py3/bin/thisroot.sh

#source geant4
source /opt/geant4/v10.5.0/bin/geant4.sh

#add gsl to library path
export LD_LIBRARY_PATH=/opt/gsl/gsl-2.6/lib/:$LD_LIBRARY_PATH
