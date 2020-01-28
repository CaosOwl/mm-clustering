#!/bin/sh
#parameters: 1: Job Name in the cluster
EOSPATH=<eospath>
EXEPATH=<exepath>
NEVENTS=<events>
EXECUTABLE=<executable>
LOCALPATH="$PWD"

#make personal folder
TJOBNAME=$1
mkdir $TJOBNAME
cd $TJOBNAME

#link mapping
ln -fs $EXEPATH/multiplexingmaps .

#copy all relevant file to exepath
cp -R -u -p -v $EXEPATH/$EXECUTABLE .
cp -R -u -p -v $EXEPATH/setup.sh .
cp -R -u -p -v $EXEPATH/config.cfg .


#source setup skript for enviroment building
source $EXEPATH/setup.sh

# run NA64 G4 MC

echo "Phase 1: run executable"
./$EXECUTABLE --config config.cfg --events $NEVENTS


echo "Phase 2: MOVING OUTPUT IN PATH: $EOSPATH"
rm $EXECUTABLE
rm -f multiplexingmaps setup.sh config.cfg
mkdir -p $EOSPATH/$TJOBNAME
cd ..

cp -R -u -p $EXEPATH/$TJOBNAME $EOSPATH/

echo "Phase 3: cleaning"

rm -rf $EXEPATH/$TJOBNAME
