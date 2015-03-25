1) Make the master send out a list of potential new masters. 
2) Make the workers receive a list of new masters
3) Make the workers elect a new master given that list
## After making MW_Run_1 tolerate failing masters, it shouldn't be too hard to 
make MW_Run_2 tolerate it, since the code is modularized