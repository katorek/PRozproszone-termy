# PRozproszone-termy
## Compiling
* Compile with script `cc` or
* Manual `mpicc termy.c -o termy -lpthread`  
## Running
* `mpirun -n X termy Y`  
where X is number of processes  
Y - max number of people in 1 changing room

## Change variables
Most variables are in definitions.h  
Edit it before compiling
