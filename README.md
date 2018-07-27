Simulated Annealing Algorithm for Floorplan
====                                                                   
 Operations:                                                        
 - Accepts data files containing:                                   
   area size, area constraints, orientation constraint      
                                                                      
   Example invocation:                                                
                                                                      
   Text mode:                                                         
   - floorplan test.dat                                               
   - e.g. context of test.dat                                         
                                                                      
     #numbers 3          // number of modules                         
     #p 0.5              // p <= q                                    
     #q 2                // r <= s                                    
     16 0.5 0.5 0        // Area, r, s, orientation                   
     10 1   2   1                                                     
     16 1   1   1                                                     
     #end                                                             

Reference:

- Rob A. Rutenbar, "Simulated Annealing Algorithms: An Overview," IEEE Circuit and Devices Magazine, pp. 19-26, Jan. 1989.
- Massound Pedram and Bryan Preas, "Benchmarks for general cell floorplanning," Computer Science Laboratory, Xerox Palo Alto Research Center, Palo Alto, CA 94304
-	Mathew J. Conway, "The SUIT version 2.3 Reference Manual," Computer Science Department, University of Virginia, 1992.
-	Mattew Conway, Randy Pausch, Kimberly Passarella, "A Tutorial for SUIT*The Simple User Interface Toolkit," Computer Science Department, University of Virginia, 1992.
