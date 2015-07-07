# ParallelAndDistributed
Labs for CS5460 -- Parallel and Distributed Systems -- Spring 2015. Collaboration between Faiyam Rahman (www.faiyamrahman.com) and Oliver Hoffman

**Every Lab has an associated report in pages/pdf. You can read the report for a detailed analysis of the code and mathematics involved with each lab**

## Lab 1
**Part 1**: The first coding part of this involved writing a parallel "Hello World" program. 

**Part 2**: The second coding part of this involved profiling the parallel computation properties of the AWS machines on which we hosted our code. The report includes graphs and analysis of the latency and bandwidth of the AWS servers. 

## Lab 2 and 3
Labs 2 and 3 were a 2 lab process of building a Master-Worker API. Using the third party library MPI, we build an API in C that anyone can use to implement their Master-Worker programs, without having to worry about the details of the Master-Worker paradigm. 

In lab 2, we got a basic API up and running. 

In Lab 3, we made the API robust to multiple worker and master failures. Lab 3 also includes code that tests the API by using it to factor large numbers. 

**Run Instructions**:

To compile and run test.c with 4 processors:

    - mpicc api.c test.c -o test
    
    - mpirun -np 4 test
    
    
## Lab 4

**Part 1**: The meat of this section is the report, which involves a detailed mathematical analysis of the Computation to Communication ratio in applying some transformational stencil to a square domain. We also present a mathematical model for the commmunication cost in a message passing computation. 

**Part 2**: In this part of the lab, we wrote a C program that reads in a stencil and image, applies the stencil to the image, and then spits out the result as a JPG. 
